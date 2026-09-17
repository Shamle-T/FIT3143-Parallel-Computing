/*
 * FIT3143 Lab 2 - Task 3 (Task 1 instrumentation)
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
 * - Supports cyclic and block workload distribution
 * - Each process performs independent primality tests
 * - Local prime lists are gathered at Rank 0
 * - Rank 0 sorts and writes the final prime list
 * - Separately measures broadcast, computation, collection and root output
 * - Uses MPI_Wtime() for wall-clock timing
 *
 * Build:
 *   mpicc -std=c11 -O3 -Wall -Wextra -pedantic task1_task3.c \
 *       -o task1_task3 -lm
 *
 * Examples:
 *
 *   mpirun -np 4 ./task1 100
 *
 *   mpirun -np 4 ./task1 10000000 --strategy cyclic
 *
 *   mpirun -np 4 ./task1 10000000 --strategy block
 *
 *   mpirun -np 4 ./task1 10000000 --strategy cyclic --benchmark
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

typedef enum {
    STRATEGY_CYCLIC = 0,
    STRATEGY_BLOCK = 1
} WorkStrategy;

typedef struct {
    uint64_t *data;
    size_t size;
    size_t capacity;
} PrimeVector;


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
    size_t odd_prime_count
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
    WorkStrategy strategy = STRATEGY_CYCLIC;
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

    double broadcast_start = MPI_Wtime();

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

    double broadcast_time = MPI_Wtime() - broadcast_start;


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

    /* Include count gathering, root receive preparation and MPI_Gatherv. */
    double collection_start = MPI_Wtime();


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

    double collection_time = MPI_Wtime() - collection_start;


    /*
     * Individual ranks no longer need their local prime list.
     */
    vector_free(&local_primes);


    /* ----------------------------------------------------- */
    /* Rank 0 sorts and writes the complete result           */
    /* ----------------------------------------------------- */

    double root_postprocess_time = 0.0;

    if (rank == 0) {

        double root_postprocess_start = MPI_Wtime();

        /*
         * Cyclic partitioning produces locally sorted lists,
         * but gathering them rank-by-rank does not produce
         * one globally sorted list.
         *
         * Therefore Rank 0 performs a final global sort.
         */
        if (total_odd_primes > 1) {

            qsort(
                all_odd_primes,
                total_odd_primes,
                sizeof(*all_odd_primes),
                compare_uint64
            );
        }


        /*
         * File output is intentionally inside the timed
         * region because the Lab rubric considers the
         * complete operation, including file writing.
         */
        write_output(
            limit,
            all_odd_primes,
            total_odd_primes
        );

        root_postprocess_time =
            MPI_Wtime() - root_postprocess_start;
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

    double maximum_broadcast_time = 0.0;
    double maximum_collection_time = 0.0;
    double maximum_root_postprocess_time = 0.0;

    double minimum_computation_time = 0.0;
    double maximum_computation_time = 0.0;
    double sum_computation_time = 0.0;


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
        &broadcast_time,
        &maximum_broadcast_time,
        1,
        MPI_DOUBLE,
        MPI_MAX,
        0,
        MPI_COMM_WORLD
    );

    MPI_Reduce(
        &collection_time,
        &maximum_collection_time,
        1,
        MPI_DOUBLE,
        MPI_MAX,
        0,
        MPI_COMM_WORLD
    );

    MPI_Reduce(
        &root_postprocess_time,
        &maximum_root_postprocess_time,
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
            "Broadcast time (max): %.9f seconds\n",
            maximum_broadcast_time
        );

        printf(
            "Result collection time (max): %.9f seconds\n",
            maximum_collection_time
        );

        printf(
            "Communication time: %.9f seconds\n",
            maximum_broadcast_time + maximum_collection_time
        );

        printf(
            "Root sort/output time: %.9f seconds\n",
            maximum_root_postprocess_time
        );

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
            "  %s <n> [--strategy cyclic|block] [--benchmark]\n",
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


    *strategy = STRATEGY_CYCLIC;
    *benchmark_mode = 0;


    for (int i = 2; i < argc; ++i) {

        if (strcmp(argv[i], "--benchmark") == 0) {

            *benchmark_mode = 1;
        }

        else if (strcmp(argv[i], "--strategy") == 0) {

            if (i + 1 >= argc) {

                fprintf(
                    stderr,
                    "Error: --strategy requires cyclic or block.\n"
                );

                return 0;
            }

            ++i;


            if (strcmp(argv[i], "cyclic") == 0) {

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


    /*
     * Same basic optimisation as the serial implementation:
     * only test odd divisors up to sqrt(candidate).
     */
    uint64_t divisor_limit =
        (uint64_t)sqrt(
            (double)candidate
        );


    for (uint64_t divisor = 3;
         divisor <= divisor_limit;
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
    /* Cyclic distribution                                   */
    /* ----------------------------------------------------- */

    if (strategy == STRATEGY_CYCLIC) {

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
/* Output                                                    */
/* --------------------------------------------------------- */

static void write_output(
    uint64_t limit,
    const uint64_t *odd_primes,
    size_t odd_prime_count
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


    /*
     * 2 is the only even prime.
     */
    if (limit > 2) {

        if (fprintf(file, "2\n") < 0) {

            fclose(file);

            MPI_Abort(
                MPI_COMM_WORLD,
                EXIT_FAILURE
            );

            return;
        }
    }


    for (size_t i = 0;
         i < odd_prime_count;
         ++i) {

        if (fprintf(
                file,
                "%" PRIu64 "\n",
                odd_primes[i]) < 0) {

            fclose(file);

            MPI_Abort(
                MPI_COMM_WORLD,
                EXIT_FAILURE
            );

            return;
        }
    }


    if (fclose(file) == EOF) {

        MPI_Abort(
            MPI_COMM_WORLD,
            EXIT_FAILURE
        );
    }
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
        default:
            return "cyclic";
    }
}
