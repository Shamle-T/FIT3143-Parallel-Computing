/*
 * FIT3143 Lab 2 - Task 1
 * Prime Search using Open MPI
 *
 * Team members:
 * - Savin Vindiv De Alwis, 35221631, sdea0018@student.monash.edu
 * - Willwara Arachchilage Shamle Imal Thilaksiri, 35512075,
 *   wthi0003@student.monash.edu
 *
 * Features:
 * - Rank 0 reads n from the command line
 * - n is broadcast to all MPI processes
 * - Odd candidate numbers are distributed between MPI processes
 * - Supports block-cyclic, cyclic, and block workload distribution
 * - Each process performs independent primality tests
 * - Local prime lists are gathered at Rank 0
 * - Rank 0 performs a k-way merge and writes the final prime list
 * - Uses MPI_Wtime() for wall-clock timing
 * - Reports computation-time imbalance between ranks
 *
 * Build:
 *   mpicc -std=c11 -O3 -Wall -Wextra -pedantic task1.c -o task1 -lm
 *
 * Examples:
 *
 *   mpirun -np 4 ./task1 100
 *
 *   mpirun -np 4 ./task1 10000000 --strategy block-cyclic
 *
 *   mpirun -np 4 ./task1 10000000 --strategy block
 *
 *   mpirun -np 4 ./task1 10000000 --strategy block-cyclic --benchmark
 */

#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <math.h>
#include <mpi.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define OUTPUT_FILE "task1_mpi_primes.txt"
#define TERMINAL_OUTPUT_LIMIT 100
#define INITIAL_VECTOR_CAPACITY 1024
#define OUTPUT_BUFFER_SIZE (1U << 20)
#define MPI_BLOCK_CHUNK 64U

typedef enum {
    STRATEGY_BLOCK_CYCLIC = 0,
    STRATEGY_CYCLIC = 1,
    STRATEGY_BLOCK = 2
} WorkStrategy;

typedef struct {
    uint64_t *data;
    size_t size;
    size_t capacity;
} PrimeVector;

typedef struct {
    uint64_t value;
    int rank;
    int local_position;
} MergeNode;


/* --------------------------------------------------------- */
/* Function declarations                                     */
/* --------------------------------------------------------- */

static int parse_options(
    int argc,
    char **argv,
    uint64_t *limit,
    WorkStrategy *strategy,
    int *benchmark_mode
);

static int parse_uint64(
    const char *text,
    uint64_t *value
);

static const char *strategy_name(
    WorkStrategy strategy
);

static int is_prime_by_trial_division(
    uint64_t candidate
);

static int vector_init(
    PrimeVector *vector
);

static int vector_push(
    PrimeVector *vector,
    uint64_t value
);

static void vector_free(
    PrimeVector *vector
);

static int search_local(
    uint64_t limit,
    int rank,
    int world_size,
    WorkStrategy strategy,
    PrimeVector *local_primes,
    uint64_t *tested_candidates
);

static int compare_uint64(
    const void *a,
    const void *b
);

static void write_output(
    uint64_t limit,
    const uint64_t *odd_primes,
    size_t odd_prime_count,
    const int *receive_counts,
    const int *displacements,
    int world_size
);

static void heap_push(
    MergeNode *heap,
    int *heap_size,
    MergeNode node
);

static MergeNode heap_pop(
    MergeNode *heap,
    int *heap_size
);

static int merge_node_less(
    MergeNode left,
    MergeNode right
);

static void print_small_result(
    uint64_t limit,
    const uint64_t *odd_primes,
    size_t odd_prime_count
);


/* --------------------------------------------------------- */
/* Main                                                      */
/* --------------------------------------------------------- */

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    int rank;
    int world_size;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);


    /* ----------------------------------------------------- */
    /* Rank 0 reads program options                          */
    /* ----------------------------------------------------- */

    uint64_t limit = 0;
    WorkStrategy strategy = STRATEGY_BLOCK_CYCLIC;
    int benchmark_mode = 0;
    int options_ok = 1;

    if (rank == 0) {
        options_ok = parse_options(
            argc,
            argv,
            &limit,
            &strategy,
            &benchmark_mode
        );
    }


    /*
     * First tell all processes whether the command-line
     * arguments were valid.
     */
    MPI_Bcast(
        &options_ok,
        1,
        MPI_INT,
        0,
        MPI_COMM_WORLD
    );

    if (!options_ok) {
        MPI_Finalize();
        return EXIT_FAILURE;
    }


    /*
     * Synchronize before measuring the complete parallel
     * operation.
     */
    MPI_Barrier(MPI_COMM_WORLD);

    double total_start = MPI_Wtime();


    /* ----------------------------------------------------- */
    /* Broadcast input and configuration                     */
    /* ----------------------------------------------------- */

    MPI_Bcast(
        &limit,
        1,
        MPI_UINT64_T,
        0,
        MPI_COMM_WORLD
    );

    int strategy_value = (int)strategy;

    MPI_Bcast(
        &strategy_value,
        1,
        MPI_INT,
        0,
        MPI_COMM_WORLD
    );

    strategy = (WorkStrategy)strategy_value;

    MPI_Bcast(
        &benchmark_mode,
        1,
        MPI_INT,
        0,
        MPI_COMM_WORLD
    );


    /* ----------------------------------------------------- */
    /* Local prime search                                    */
    /* ----------------------------------------------------- */

    PrimeVector local_primes;

    if (!vector_init(&local_primes)) {
        fprintf(
            stderr,
            "Rank %d: failed to allocate local prime storage.\n",
            rank
        );

        MPI_Abort(
            MPI_COMM_WORLD,
            EXIT_FAILURE
        );

        return EXIT_FAILURE;
    }


    uint64_t tested_candidates = 0;

    double computation_start = MPI_Wtime();

    if (!search_local(
            limit,
            rank,
            world_size,
            strategy,
            &local_primes,
            &tested_candidates)) {

        fprintf(
            stderr,
            "Rank %d: memory allocation failed during search.\n",
            rank
        );

        vector_free(&local_primes);

        MPI_Abort(
            MPI_COMM_WORLD,
            EXIT_FAILURE
        );

        return EXIT_FAILURE;
    }

    double computation_end = MPI_Wtime();

    double computation_time =
        computation_end - computation_start;

    /*
     * Retain per-rank measurements as direct evidence that block-cyclic
     * ownership balances both candidate count and measured computation time.
     */
    double *per_rank_computation_times = NULL;
    uint64_t *per_rank_candidate_counts = NULL;

    if (rank == 0) {
        per_rank_computation_times =
            malloc(
                (size_t)world_size *
                sizeof(*per_rank_computation_times)
            );

        per_rank_candidate_counts =
            malloc(
                (size_t)world_size *
                sizeof(*per_rank_candidate_counts)
            );

        if (per_rank_computation_times == NULL ||
            per_rank_candidate_counts == NULL) {

            free(per_rank_computation_times);
            free(per_rank_candidate_counts);
            vector_free(&local_primes);
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
            return EXIT_FAILURE;
        }
    }

    MPI_Gather(
        &computation_time,
        1,
        MPI_DOUBLE,
        per_rank_computation_times,
        1,
        MPI_DOUBLE,
        0,
        MPI_COMM_WORLD
    );

    MPI_Gather(
        &tested_candidates,
        1,
        MPI_UINT64_T,
        per_rank_candidate_counts,
        1,
        MPI_UINT64_T,
        0,
        MPI_COMM_WORLD
    );


    /*
     * Measure result communication separately.  This allows Task 3 to
     * distinguish the scalable search kernel from collective communication
     * and root-only post-processing when applying Amdahl's Law.
     */
    double result_gather_start = MPI_Wtime();


    /* ----------------------------------------------------- */
    /* Gather number of primes found by each process         */
    /* ----------------------------------------------------- */

    if (local_primes.size > INT_MAX) {
        fprintf(
            stderr,
            "Rank %d found too many primes for MPI_Gatherv.\n",
            rank
        );

        vector_free(&local_primes);

        MPI_Abort(
            MPI_COMM_WORLD,
            EXIT_FAILURE
        );

        return EXIT_FAILURE;
    }


    int local_count = (int)local_primes.size;

    int *receive_counts = NULL;

    if (rank == 0) {
        receive_counts =
            malloc(
                (size_t)world_size *
                sizeof(*receive_counts)
            );

        if (receive_counts == NULL) {
            fprintf(
                stderr,
                "Rank 0: unable to allocate receive count array.\n"
            );

            vector_free(&local_primes);

            MPI_Abort(
                MPI_COMM_WORLD,
                EXIT_FAILURE
            );

            return EXIT_FAILURE;
        }
    }


    MPI_Gather(
        &local_count,
        1,
        MPI_INT,
        receive_counts,
        1,
        MPI_INT,
        0,
        MPI_COMM_WORLD
    );


    /* ----------------------------------------------------- */
    /* Rank 0 calculates MPI_Gatherv displacements           */
    /* ----------------------------------------------------- */

    int *displacements = NULL;

    uint64_t *all_odd_primes = NULL;

    size_t total_odd_primes = 0;


    if (rank == 0) {

        displacements =
            malloc(
                (size_t)world_size *
                sizeof(*displacements)
            );

        if (displacements == NULL) {
            fprintf(
                stderr,
                "Rank 0: unable to allocate displacement array.\n"
            );

            free(receive_counts);
            vector_free(&local_primes);

            MPI_Abort(
                MPI_COMM_WORLD,
                EXIT_FAILURE
            );

            return EXIT_FAILURE;
        }


        int running_offset = 0;

        for (int process = 0;
             process < world_size;
             ++process) {

            displacements[process] =
                running_offset;

            if (receive_counts[process] >
                INT_MAX - running_offset) {

                fprintf(
                    stderr,
                    "Too many primes for MPI_Gatherv.\n"
                );

                free(receive_counts);
                free(displacements);
                vector_free(&local_primes);

                MPI_Abort(
                    MPI_COMM_WORLD,
                    EXIT_FAILURE
                );

                return EXIT_FAILURE;
            }

            running_offset +=
                receive_counts[process];
        }

        total_odd_primes =
            (size_t)running_offset;


        if (total_odd_primes > 0) {

            all_odd_primes =
                malloc(
                    total_odd_primes *
                    sizeof(*all_odd_primes)
                );

            if (all_odd_primes == NULL) {

                fprintf(
                    stderr,
                    "Rank 0: unable to allocate final prime array.\n"
                );

                free(receive_counts);
                free(displacements);
                vector_free(&local_primes);

                MPI_Abort(
                    MPI_COMM_WORLD,
                    EXIT_FAILURE
                );

                return EXIT_FAILURE;
            }
        }
    }


    /* ----------------------------------------------------- */
    /* Gather all locally discovered primes at Rank 0        */
    /* ----------------------------------------------------- */

    MPI_Gatherv(
        local_primes.data,
        local_count,
        MPI_UINT64_T,

        all_odd_primes,
        receive_counts,
        displacements,
        MPI_UINT64_T,

        0,
        MPI_COMM_WORLD
    );

    double local_result_gather_time =
        MPI_Wtime() - result_gather_start;


    /*
     * Individual ranks no longer need their local prime list.
     */
    vector_free(&local_primes);


    /* ----------------------------------------------------- */
    /* Rank 0 sorts and writes the complete result           */
    /* ----------------------------------------------------- */

    double root_sort_output_time = 0.0;

    if (rank == 0) {

        double root_sort_output_start = MPI_Wtime();

        /*
         * Each rank contributes an ascending prime list.  A k-way merge
         * produces globally ordered output in O(m log p), instead of
         * a full root qsort in O(m log m), where m is the number of primes
         * and p is the number of MPI processes.
         */
        write_output(
            limit,
            all_odd_primes,
            total_odd_primes,
            receive_counts,
            displacements,
            world_size
        );

        root_sort_output_time =
            MPI_Wtime() - root_sort_output_start;
    }


    /*
     * Ensure all ranks wait until Rank 0 completes sorting
     * and file output.
     */
    MPI_Barrier(MPI_COMM_WORLD);

    double total_end = MPI_Wtime();

    double local_total_time =
        total_end - total_start;


    /* ----------------------------------------------------- */
    /* Collect timing information                            */
    /* ----------------------------------------------------- */

    double max_total_time = 0.0;

    double minimum_computation_time = 0.0;
    double maximum_computation_time = 0.0;
    double sum_computation_time = 0.0;
    double maximum_result_gather_time = 0.0;


    MPI_Reduce(
        &local_total_time,
        &max_total_time,
        1,
        MPI_DOUBLE,
        MPI_MAX,
        0,
        MPI_COMM_WORLD
    );


    MPI_Reduce(
        &computation_time,
        &minimum_computation_time,
        1,
        MPI_DOUBLE,
        MPI_MIN,
        0,
        MPI_COMM_WORLD
    );


    MPI_Reduce(
        &local_result_gather_time,
        &maximum_result_gather_time,
        1,
        MPI_DOUBLE,
        MPI_MAX,
        0,
        MPI_COMM_WORLD
    );


    MPI_Reduce(
        &computation_time,
        &maximum_computation_time,
        1,
        MPI_DOUBLE,
        MPI_MAX,
        0,
        MPI_COMM_WORLD
    );


    MPI_Reduce(
        &computation_time,
        &sum_computation_time,
        1,
        MPI_DOUBLE,
        MPI_SUM,
        0,
        MPI_COMM_WORLD
    );


    uint64_t minimum_candidates = 0;
    uint64_t maximum_candidates = 0;
    uint64_t total_candidates = 0;


    MPI_Reduce(
        &tested_candidates,
        &minimum_candidates,
        1,
        MPI_UINT64_T,
        MPI_MIN,
        0,
        MPI_COMM_WORLD
    );


    MPI_Reduce(
        &tested_candidates,
        &maximum_candidates,
        1,
        MPI_UINT64_T,
        MPI_MAX,
        0,
        MPI_COMM_WORLD
    );


    MPI_Reduce(
        &tested_candidates,
        &total_candidates,
        1,
        MPI_UINT64_T,
        MPI_SUM,
        0,
        MPI_COMM_WORLD
    );


    /* ----------------------------------------------------- */
    /* Rank 0 prints benchmark / program summary             */
    /* ----------------------------------------------------- */

    if (rank == 0) {

        size_t total_prime_count =
            total_odd_primes +
            ((limit > 2) ? 1U : 0U);


        double average_computation_time =
            sum_computation_time /
            (double)world_size;


        printf("\n");
        printf("Implementation: Open MPI\n");
        printf("Input limit: %" PRIu64 "\n", limit);
        printf("MPI processes: %d\n", world_size);
        printf(
            "Workload strategy: %s\n",
            strategy_name(strategy)
        );

        printf(
            "Prime count: %zu\n",
            total_prime_count
        );

        printf(
            "Candidates tested: %" PRIu64 "\n",
            total_candidates
        );

        printf(
            "Candidates per rank (min/max): "
            "%" PRIu64 " / %" PRIu64 "\n",
            minimum_candidates,
            maximum_candidates
        );

        printf(
            "Computation time (min): %.9f seconds\n",
            minimum_computation_time
        );

        printf(
            "Computation time (avg): %.9f seconds\n",
            average_computation_time
        );

        printf(
            "Computation time (max): %.9f seconds\n",
            maximum_computation_time
        );

        printf(
            "Maximum result-gather time: %.9f seconds\n",
            maximum_result_gather_time
        );

        printf(
            "Root merge + file-write time: %.9f seconds\n",
            root_sort_output_time
        );

        printf("Per-rank computation balance:\n");
        for (int process = 0;
             process < world_size;
             ++process) {

            printf(
                "  Rank %d: %" PRIu64
                " candidates, %.9f seconds\n",
                process,
                per_rank_candidate_counts[process],
                per_rank_computation_times[process]
            );
        }

        if (minimum_computation_time > 0.0) {
            printf(
                "Computation imbalance (max/min): %.6f\n",
                maximum_computation_time /
                minimum_computation_time
            );
        }

        printf(
            "Overall wall-clock time: %.9f seconds\n",
            max_total_time
        );

        printf(
            "Output file: %s\n",
            OUTPUT_FILE
        );


        /*
         * For tiny correctness tests, display the primes
         * on the terminal as well.
         */
        if (!benchmark_mode &&
            limit < TERMINAL_OUTPUT_LIMIT) {

            /* Only tiny interactive examples need an in-memory sorted copy. */
            if (total_odd_primes > 1) {
                qsort(
                    all_odd_primes,
                    total_odd_primes,
                    sizeof(*all_odd_primes),
                    compare_uint64
                );
            }

            print_small_result(
                limit,
                all_odd_primes,
                total_odd_primes
            );
        }
    }


    free(all_odd_primes);
    free(receive_counts);
    free(displacements);
    free(per_rank_computation_times);
    free(per_rank_candidate_counts);


    MPI_Finalize();

    return EXIT_SUCCESS;
}


/* --------------------------------------------------------- */
/* Command-line parsing                                      */
/* --------------------------------------------------------- */

static int parse_options(
    int argc,
    char **argv,
    uint64_t *limit,
    WorkStrategy *strategy,
    int *benchmark_mode
)
{
    if (argc < 2) {

        fprintf(
            stderr,
            "Usage:\n"
            "  %s <n> [--strategy block-cyclic|cyclic|block] "
            "[--benchmark]\n",
            argv[0]
        );

        return 0;
    }


    if (!parse_uint64(argv[1], limit)) {

        fprintf(
            stderr,
            "Error: n must be a non-negative integer.\n"
        );

        return 0;
    }


    *strategy = STRATEGY_BLOCK_CYCLIC;
    *benchmark_mode = 0;


    for (int i = 2; i < argc; ++i) {

        if (strcmp(argv[i], "--benchmark") == 0) {

            *benchmark_mode = 1;
        }

        else if (strcmp(argv[i], "--strategy") == 0) {

            if (i + 1 >= argc) {

                fprintf(
                    stderr,
                    "Error: --strategy requires block-cyclic, cyclic, "
                    "or block.\n"
                );

                return 0;
            }

            ++i;


            if (strcmp(argv[i], "block-cyclic") == 0) {

                *strategy =
                    STRATEGY_BLOCK_CYCLIC;
            }

            else if (strcmp(argv[i], "cyclic") == 0) {

                *strategy =
                    STRATEGY_CYCLIC;
            }

            else if (strcmp(argv[i], "block") == 0) {

                *strategy =
                    STRATEGY_BLOCK;
            }

            else {

                fprintf(
                    stderr,
                    "Error: unknown strategy '%s'.\n",
                    argv[i]
                );

                return 0;
            }
        }

        else {

            fprintf(
                stderr,
                "Error: unknown option '%s'.\n",
                argv[i]
            );

            return 0;
        }
    }


    return 1;
}


static int parse_uint64(
    const char *text,
    uint64_t *value
)
{
    char *end = NULL;

    while (isspace((unsigned char)*text)) {
        ++text;
    }

    if (*text == '-') {
        return 0;
    }


    errno = 0;

    uintmax_t parsed =
        strtoumax(
            text,
            &end,
            10
        );


    if (text == end ||
        errno == ERANGE ||
        parsed > UINT64_MAX) {

        return 0;
    }


    while (isspace((unsigned char)*end)) {
        ++end;
    }


    if (*end != '\0') {
        return 0;
    }


    *value =
        (uint64_t)parsed;

    return 1;
}


/* --------------------------------------------------------- */
/* Prime testing                                             */
/* --------------------------------------------------------- */

static int is_prime_by_trial_division(
    uint64_t candidate
)
{
    if (candidate < 2) {
        return 0;
    }

    if (candidate == 2) {
        return 1;
    }

    if (candidate % 2 == 0) {
        return 0;
    }


    /* Equivalent to divisor <= sqrt(candidate), without floating-point
       rounding or divisor * divisor overflow for large uint64_t inputs. */
    for (uint64_t divisor = 3;
         divisor <= candidate / divisor;
         divisor += 2) {

        if (candidate % divisor == 0) {
            return 0;
        }
    }


    return 1;
}


/* --------------------------------------------------------- */
/* Local workload                                            */
/* --------------------------------------------------------- */

static int search_local(
    uint64_t limit,
    int rank,
    int world_size,
    WorkStrategy strategy,
    PrimeVector *local_primes,
    uint64_t *tested_candidates
)
{
    /*
     * Odd candidates are:
     *
     * index 0 -> 3
     * index 1 -> 5
     * index 2 -> 7
     * ...
     */
    uint64_t odd_count =
        (limit > 2)
            ? (limit - 2) / 2
            : 0;


    *tested_candidates = 0;


    /* ----------------------------------------------------- */
    /* Block-cyclic distribution                             */
    /* ----------------------------------------------------- */

    if (strategy == STRATEGY_BLOCK_CYCLIC) {

        /*
         * Assign short contiguous chunks round-robin.  Pure one-candidate
         * cyclic ownership can lock a rank onto a cheap residue class when
         * the process count has an odd factor (for example, one of three
         * ranks receives every multiple of 3).  A 64-candidate chunk contains
         * a representative mix of small-divisor residues, while successive
         * chunks still spread low and high candidate magnitudes across ranks.
         */
        uint64_t chunk = (uint64_t)MPI_BLOCK_CHUNK;
        uint64_t cycle = chunk * (uint64_t)world_size;
        uint64_t first = (uint64_t)rank * chunk;

        for (uint64_t block_start = first;
             block_start < odd_count;
             block_start += cycle) {

            uint64_t block_end = block_start + chunk;

            if (block_end > odd_count) {
                block_end = odd_count;
            }

            for (uint64_t index = block_start;
                 index < block_end;
                 ++index) {

                uint64_t candidate = 2 * index + 3;

                ++(*tested_candidates);

                if (is_prime_by_trial_division(candidate) &&
                    !vector_push(local_primes, candidate)) {

                    return 0;
                }
            }
        }
    }


    /* ----------------------------------------------------- */
    /* Single-candidate cyclic distribution (comparison)     */
    /* ----------------------------------------------------- */

    else if (strategy == STRATEGY_CYCLIC) {

        /*
         * Example with four ranks:
         *
         * Rank 0 -> indices 0, 4, 8, ...
         * Rank 1 -> indices 1, 5, 9, ...
         * Rank 2 -> indices 2, 6, 10, ...
         * Rank 3 -> indices 3, 7, 11, ...
         */
        for (uint64_t index = (uint64_t)rank;
             index < odd_count;
             index += (uint64_t)world_size) {

            uint64_t candidate =
                2 * index + 3;


            ++(*tested_candidates);


            if (is_prime_by_trial_division(candidate)) {

                if (!vector_push(
                        local_primes,
                        candidate)) {

                    return 0;
                }
            }
        }
    }


    /* ----------------------------------------------------- */
    /* Block distribution                                    */
    /* ----------------------------------------------------- */

    else {

        uint64_t process_count =
            (uint64_t)world_size;

        uint64_t base =
            odd_count / process_count;

        uint64_t remainder =
            odd_count % process_count;


        /*
         * First 'remainder' ranks receive one extra
         * candidate.
         */
        uint64_t local_work =
            base +
            (
                (uint64_t)rank < remainder
                    ? 1
                    : 0
            );


        uint64_t start_index =
            (uint64_t)rank * base +
            (
                (uint64_t)rank < remainder
                    ? (uint64_t)rank
                    : remainder
            );


        uint64_t end_index =
            start_index + local_work;


        for (uint64_t index = start_index;
             index < end_index;
             ++index) {

            uint64_t candidate =
                2 * index + 3;


            ++(*tested_candidates);


            if (is_prime_by_trial_division(candidate)) {

                if (!vector_push(
                        local_primes,
                        candidate)) {

                    return 0;
                }
            }
        }
    }


    return 1;
}


/* --------------------------------------------------------- */
/* Dynamic array                                             */
/* --------------------------------------------------------- */

static int vector_init(
    PrimeVector *vector
)
{
    vector->size = 0;

    vector->capacity =
        INITIAL_VECTOR_CAPACITY;

    vector->data =
        malloc(
            vector->capacity *
            sizeof(*vector->data)
        );


    return vector->data != NULL;
}


static int vector_push(
    PrimeVector *vector,
    uint64_t value
)
{
    if (vector->size ==
        vector->capacity) {

        size_t new_capacity =
            vector->capacity * 2;


        if (new_capacity <
            vector->capacity) {

            return 0;
        }


        uint64_t *new_data =
            realloc(
                vector->data,
                new_capacity *
                sizeof(*new_data)
            );


        if (new_data == NULL) {
            return 0;
        }


        vector->data =
            new_data;

        vector->capacity =
            new_capacity;
    }


    vector->data[
        vector->size++
    ] = value;


    return 1;
}


static void vector_free(
    PrimeVector *vector
)
{
    free(vector->data);

    vector->data = NULL;
    vector->size = 0;
    vector->capacity = 0;
}


/* --------------------------------------------------------- */
/* Sorting                                                   */
/* --------------------------------------------------------- */

static int compare_uint64(
    const void *a,
    const void *b
)
{
    uint64_t first =
        *(const uint64_t *)a;

    uint64_t second =
        *(const uint64_t *)b;


    if (first < second) {
        return -1;
    }

    if (first > second) {
        return 1;
    }

    return 0;
}


/* --------------------------------------------------------- */
/* Ordered output                                            */
/* --------------------------------------------------------- */

static void write_output(
    uint64_t limit,
    const uint64_t *odd_primes,
    size_t odd_prime_count,
    const int *receive_counts,
    const int *displacements,
    int world_size
)
{
    FILE *file =
        fopen(
            OUTPUT_FILE,
            "w"
        );


    if (file == NULL) {

        perror(
            "Unable to open output file"
        );

        MPI_Abort(
            MPI_COMM_WORLD,
            EXIT_FAILURE
        );

        return;
    }

    /* Use the same explicit 1 MiB file buffer as the other implementations. */
    char *output_buffer =
        malloc(OUTPUT_BUFFER_SIZE);

    if (output_buffer != NULL) {
        (void)setvbuf(
            file,
            output_buffer,
            _IOFBF,
            OUTPUT_BUFFER_SIZE
        );
    }


    MergeNode *heap =
        malloc(
            (size_t)world_size *
            sizeof(*heap)
        );

    if (heap == NULL && world_size > 0) {
        free(output_buffer);
        fclose(file);
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        return;
    }

    int heap_size = 0;
    size_t emitted_odd_primes = 0;
    int write_ok = 1;

    if (limit > 2 && fprintf(file, "2\n") < 0) {
        write_ok = 0;
    }

    for (int process = 0;
         process < world_size;
         ++process) {

        if (receive_counts[process] > 0) {

            MergeNode node = {
                odd_primes[displacements[process]],
                process,
                0
            };

            heap_push(heap, &heap_size, node);
        }
    }

    while (write_ok && heap_size > 0) {

        MergeNode node =
            heap_pop(heap, &heap_size);

        if (fprintf(
                file,
                "%" PRIu64 "\n",
                node.value) < 0) {

            write_ok = 0;
            break;
        }

        ++emitted_odd_primes;

        int next_position =
            node.local_position + 1;

        if (next_position < receive_counts[node.rank]) {

            MergeNode next_node = {
                odd_primes[
                    displacements[node.rank] +
                    next_position
                ],
                node.rank,
                next_position
            };

            heap_push(heap, &heap_size, next_node);
        }
    }

    if (emitted_odd_primes != odd_prime_count) {
        write_ok = 0;
    }

    if (fclose(file) == EOF) {
        write_ok = 0;
    }

    free(heap);
    free(output_buffer);

    if (!write_ok) {
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }
}


static void heap_push(
    MergeNode *heap,
    int *heap_size,
    MergeNode node
)
{
    int child = (*heap_size)++;

    heap[child] = node;

    while (child > 0) {

        int parent =
            (child - 1) / 2;

        if (!merge_node_less(
                heap[child],
                heap[parent])) {

            break;
        }

        MergeNode temporary = heap[parent];
        heap[parent] = heap[child];
        heap[child] = temporary;
        child = parent;
    }
}


static MergeNode heap_pop(
    MergeNode *heap,
    int *heap_size
)
{
    MergeNode minimum = heap[0];
    MergeNode last = heap[--(*heap_size)];

    if (*heap_size == 0) {
        return minimum;
    }

    heap[0] = last;

    int parent = 0;

    while (1) {

        int left_child = 2 * parent + 1;
        int right_child = left_child + 1;
        int smallest = parent;

        if (left_child < *heap_size &&
            merge_node_less(
                heap[left_child],
                heap[smallest])) {

            smallest = left_child;
        }

        if (right_child < *heap_size &&
            merge_node_less(
                heap[right_child],
                heap[smallest])) {

            smallest = right_child;
        }

        if (smallest == parent) {
            break;
        }

        MergeNode temporary = heap[parent];
        heap[parent] = heap[smallest];
        heap[smallest] = temporary;
        parent = smallest;
    }

    return minimum;
}


static int merge_node_less(
    MergeNode left,
    MergeNode right
)
{
    if (left.value != right.value) {
        return left.value < right.value;
    }

    return left.rank < right.rank;
}


static void print_small_result(
    uint64_t limit,
    const uint64_t *odd_primes,
    size_t odd_prime_count
)
{
    printf(
        "Primes less than %" PRIu64 ":",
        limit
    );


    if (limit > 2) {
        printf(" 2");
    }


    for (size_t i = 0;
         i < odd_prime_count;
         ++i) {

        printf(
            " %" PRIu64,
            odd_primes[i]
        );
    }


    printf("\n");
}


static const char *strategy_name(
    WorkStrategy strategy
)
{
    switch (strategy) {

        case STRATEGY_BLOCK:
            return "block";

        case STRATEGY_CYCLIC:
            return "cyclic";

        case STRATEGY_BLOCK_CYCLIC:
        default:
            return "block-cyclic";
    }
}
