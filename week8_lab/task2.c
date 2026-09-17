/*
 * FIT3143 Lab 2 - Task 2: Hybrid Open MPI + OpenMP prime search
 *
 * Finds every prime strictly below n using the same odd-only trial-division
 * kernel as the Week 4 implementations. Open MPI distributes candidates
 * across processes and OpenMP parallelises each process's local work.
 *
 * Workload design:
 * - Only odd candidates are tested; 2 is handled separately.
 * - MPI uses cyclic (round-robin) distribution of odd candidates. Trial
 *   division becomes more expensive as candidate values grow, so cyclic
 *   ownership gives every rank a mix of low- and high-valued candidates and
 *   avoids the imbalance of equal contiguous blocks.
 * - OpenMP uses dynamic scheduling inside each MPI process to smooth the
 *   remaining variation between cheap composite tests and expensive prime
 *   tests.
 * - Each rank compacts only its prime values before communication. Rank 0
 *   gathers these sorted local lists with MPI_Gatherv and performs a k-way
 *   merge in O(number_of_primes * log(number_of_processes)) while writing the
 *   final sorted output. This avoids gathering all candidate flags and avoids
 *   a full O(m log m) qsort on the root.
 *
 * Team members:
 * - Savin Vindiv De Alwis, 35221631, sdea0018@student.monash.edu
 * - Willwara Arachchilage Shamle Imal Thilaksiri, 35512075,
 *   wthi0003@student.monash.edu
 *
 * Build:
 *   mpicc -std=c11 -O3 -Wall -Wextra -Wpedantic -fopenmp task2.c -o task2 -lm
 *
 * Run (example: 4 MPI processes, 2 OpenMP threads per process):
 *   mpirun -np 4 ./task2 10000000 2
 *
 * Output:
 *   task2_primes.txt
 */

#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <math.h>
#include <mpi.h>
#include <omp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define ROOT_RANK 0
#define MAX_THREAD_COUNT 1024
#define OUTPUT_FILE_BASENAME "task2_primes.txt"
#define OUTPUT_BUFFER_SIZE (1U << 20)
#define OMP_DYNAMIC_CHUNK 64

typedef struct {
  uint64_t limit;
  int thread_count;
} ProgramConfig;

typedef struct {
  unsigned char *is_prime;
  uint64_t *primes;
  size_t candidate_count;
  uint64_t prime_count;
} LocalResult;

typedef struct {
  uint64_t value;
  int rank;
  int local_position;
} MergeNode;

static int parse_root_arguments(int argc, char *argv[], ProgramConfig *config);
static int parse_uint64(const char *text, const char *name, uint64_t minimum,
                        uint64_t maximum, uint64_t *value);
static size_t odd_candidate_count(uint64_t limit);
static size_t cyclic_candidate_count(size_t odd_count, int rank,
                                     int process_count);
static size_t global_odd_index(size_t local_index, int rank,
                               int process_count);
static uint64_t odd_value(size_t global_index);
static int is_prime_by_trial_division(uint64_t candidate);
static int compute_local_primes(int rank, int process_count, int thread_count,
                                LocalResult *result);
static int compact_local_primes(int rank, int process_count,
                                LocalResult *result);
static int merge_and_save_primes(const char *output_path, uint64_t limit,
                                 const uint64_t *gathered_primes,
                                 const int *receive_counts,
                                 const int *receive_displacements,
                                 int process_count,
                                 uint64_t expected_odd_prime_count);
static void heap_push(MergeNode *heap, int *heap_size, MergeNode node);
static MergeNode heap_pop(MergeNode *heap, int *heap_size);
static int merge_node_less(MergeNode left, MergeNode right);
static int all_processes_ok(int local_ok, MPI_Comm communicator);
static void free_local_result(LocalResult *result);

int main(int argc, char *argv[]) {
  const MPI_Comm communicator = MPI_COMM_WORLD;
  ProgramConfig config = {0, 0};
  LocalResult local_result = {NULL, NULL, 0, 0};
  uint64_t *rank_prime_counts = NULL;
  uint64_t *gathered_odd_primes = NULL;
  int *receive_counts = NULL;
  int *receive_displacements = NULL;
  uint64_t global_odd_prime_count = 0;
  uint64_t global_prime_count = 0;
  size_t odd_count = 0;
  int rank = -1;
  int process_count = 0;
  int provided_thread_level = MPI_THREAD_SINGLE;
  int arguments_valid = 1;
  int local_ok = 1;
  int gather_ok = 1;
  int output_ok = 1;
  int exit_code = EXIT_SUCCESS;
  double overall_start = 0.0;
  double local_compute_seconds = 0.0;
  double max_compute_seconds = 0.0;
  double local_gather_seconds = 0.0;
  double max_gather_seconds = 0.0;
  double merge_write_seconds = 0.0;
  double local_overall_seconds = 0.0;
  double max_overall_seconds = 0.0;

  if (MPI_Init_thread(&argc, &argv, MPI_THREAD_FUNNELED,
                      &provided_thread_level) != MPI_SUCCESS) {
    fputs("Error: MPI_Init_thread failed.\n", stderr);
    return EXIT_FAILURE;
  }

  MPI_Comm_rank(communicator, &rank);
  MPI_Comm_size(communicator, &process_count);

  if (provided_thread_level < MPI_THREAD_FUNNELED) {
    if (rank == ROOT_RANK) {
      fputs("Error: the MPI implementation does not provide MPI_THREAD_FUNNELED "
            "support required by this hybrid program.\n",
            stderr);
    }
    MPI_Finalize();
    return EXIT_FAILURE;
  }

  /* Align all ranks before the end-to-end wall-clock measurement. */
  MPI_Barrier(communicator);
  overall_start = MPI_Wtime();

  /* Only the root process reads the command-line configuration. */
  if (rank == ROOT_RANK) {
    arguments_valid = parse_root_arguments(argc, argv, &config);
  }
  MPI_Bcast(&arguments_valid, 1, MPI_INT, ROOT_RANK, communicator);
  if (!arguments_valid) {
    MPI_Finalize();
    return EXIT_FAILURE;
  }

  MPI_Bcast(&config.limit, 1, MPI_UINT64_T, ROOT_RANK, communicator);
  MPI_Bcast(&config.thread_count, 1, MPI_INT, ROOT_RANK, communicator);

  /* Every rank must be able to represent candidate indices with size_t. */
  local_ok = config.limit <= (uint64_t)SIZE_MAX;
  if (!all_processes_ok(local_ok, communicator)) {
    if (rank == ROOT_RANK) {
      fputs("Error: n is too large for size_t on at least one MPI process.\n",
            stderr);
    }
    exit_code = EXIT_FAILURE;
    goto cleanup;
  }

  omp_set_dynamic(0);
  odd_count = odd_candidate_count(config.limit);
  local_result.candidate_count =
      cyclic_candidate_count(odd_count, rank, process_count);

  if (local_result.candidate_count > 0) {
    local_result.is_prime = malloc(local_result.candidate_count *
                                   sizeof(*local_result.is_prime));
    local_ok = local_result.is_prime != NULL;
  }
  if (!all_processes_ok(local_ok, communicator)) {
    if (rank == ROOT_RANK) {
      fputs("Error: unable to allocate local prime flags on at least one MPI "
            "process.\n",
            stderr);
    }
    exit_code = EXIT_FAILURE;
    goto cleanup;
  }

  /*
   * Computation phase: every MPI process creates an OpenMP team and computes
   * its cyclic share of odd candidates.
   */
  {
    double compute_start = MPI_Wtime();
    local_ok = compute_local_primes(rank, process_count, config.thread_count,
                                    &local_result) &&
               compact_local_primes(rank, process_count, &local_result);
    local_compute_seconds = MPI_Wtime() - compute_start;
  }

  if (!all_processes_ok(local_ok, communicator)) {
    if (rank == ROOT_RANK) {
      fputs("Error: unable to allocate/compact local prime results on at least "
            "one MPI process.\n",
            stderr);
    }
    exit_code = EXIT_FAILURE;
    goto cleanup;
  }

  MPI_Reduce(&local_compute_seconds, &max_compute_seconds, 1, MPI_DOUBLE,
             MPI_MAX, ROOT_RANK, communicator);

  /* Rank 0 learns how many prime values each rank will contribute. */
  if (rank == ROOT_RANK) {
    rank_prime_counts =
        malloc((size_t)process_count * sizeof(*rank_prime_counts));
    receive_counts = malloc((size_t)process_count * sizeof(*receive_counts));
    receive_displacements =
        malloc((size_t)process_count * sizeof(*receive_displacements));
    local_ok = rank_prime_counts != NULL && receive_counts != NULL &&
               receive_displacements != NULL;
  } else {
    local_ok = 1;
  }

  if (!all_processes_ok(local_ok, communicator)) {
    if (rank == ROOT_RANK) {
      fputs("Error: unable to allocate root gather metadata.\n", stderr);
    }
    exit_code = EXIT_FAILURE;
    goto cleanup;
  }

  {
    double gather_start = MPI_Wtime();

    MPI_Gather(&local_result.prime_count, 1, MPI_UINT64_T, rank_prime_counts, 1,
               MPI_UINT64_T, ROOT_RANK, communicator);

    if (rank == ROOT_RANK) {
      uint64_t displacement = 0;
      int process_index;

      for (process_index = 0; process_index < process_count; ++process_index) {
        uint64_t count = rank_prime_counts[process_index];

        if (count > (uint64_t)INT_MAX || displacement > (uint64_t)INT_MAX ||
            displacement + count > (uint64_t)INT_MAX) {
          gather_ok = 0;
          break;
        }

        receive_counts[process_index] = (int)count;
        receive_displacements[process_index] = (int)displacement;
        displacement += count;
      }

      if (gather_ok) {
        global_odd_prime_count = displacement;
        if (global_odd_prime_count > 0) {
          gathered_odd_primes = malloc((size_t)global_odd_prime_count *
                                       sizeof(*gathered_odd_primes));
          if (gathered_odd_primes == NULL) {
            gather_ok = 0;
          }
        }
      }
    }

    MPI_Bcast(&gather_ok, 1, MPI_INT, ROOT_RANK, communicator);
    if (gather_ok) {
      int local_send_count = (int)local_result.prime_count;

      MPI_Gatherv(local_result.primes, local_send_count, MPI_UINT64_T,
                  gathered_odd_primes, receive_counts, receive_displacements,
                  MPI_UINT64_T, ROOT_RANK, communicator);
    }

    local_gather_seconds = MPI_Wtime() - gather_start;
  }

  if (!gather_ok) {
    if (rank == ROOT_RANK) {
      fputs("Error: gathered result exceeds MPI_Gatherv's int count/displacement "
            "range or root result allocation failed.\n",
            stderr);
    }
    exit_code = EXIT_FAILURE;
    goto cleanup;
  }

  MPI_Reduce(&local_gather_seconds, &max_gather_seconds, 1, MPI_DOUBLE, MPI_MAX,
             ROOT_RANK, communicator);

  /*
   * Each rank's compacted prime list is sorted, but cyclic MPI ownership
   * interleaves the rank lists globally. Rank 0 performs an efficient k-way
   * merge while writing the required sorted output file.
   */
  if (rank == ROOT_RANK) {
    double merge_write_start = MPI_Wtime();

    output_ok = merge_and_save_primes(
        OUTPUT_FILE_BASENAME, config.limit, gathered_odd_primes, receive_counts,
        receive_displacements, process_count, global_odd_prime_count);
    merge_write_seconds = MPI_Wtime() - merge_write_start;
    global_prime_count =
        global_odd_prime_count + (config.limit > 2 ? UINT64_C(1) : UINT64_C(0));
  }

  MPI_Bcast(&output_ok, 1, MPI_INT, ROOT_RANK, communicator);
  if (!output_ok) {
    exit_code = EXIT_FAILURE;
  }

  /* Include root merging/file output and all waiting in the end-to-end time. */
  MPI_Barrier(communicator);
  local_overall_seconds = MPI_Wtime() - overall_start;
  MPI_Reduce(&local_overall_seconds, &max_overall_seconds, 1, MPI_DOUBLE,
             MPI_MAX, ROOT_RANK, communicator);

  if (rank == ROOT_RANK && output_ok) {
    long long total_workers =
        (long long)process_count * (long long)config.thread_count;

    printf("Implementation: hybrid Open MPI + OpenMP\n");
    printf("Input limit: %" PRIu64 "\n", config.limit);
    printf("MPI processes: %d\n", process_count);
    printf("OpenMP threads per process: %d\n", config.thread_count);
    printf("Total worker threads: %lld\n", total_workers);
    printf("Prime count: %" PRIu64 "\n", global_prime_count);
    printf("Maximum computation time: %.9f seconds\n", max_compute_seconds);
    printf("Maximum gather time: %.9f seconds\n", max_gather_seconds);
    printf("Root merge + file-write time: %.9f seconds\n",
           merge_write_seconds);
    printf("Overall wall-clock time: %.9f seconds\n", max_overall_seconds);
    printf("Primes written to %s\n", OUTPUT_FILE_BASENAME);
  }

cleanup:
  free(gathered_odd_primes);
  free(receive_displacements);
  free(receive_counts);
  free(rank_prime_counts);
  free_local_result(&local_result);

  MPI_Finalize();
  return exit_code;
}

static int parse_root_arguments(int argc, char *argv[], ProgramConfig *config) {
  uint64_t thread_count = 0;

  if (argc != 3) {
    fprintf(stderr, "Usage: %s n threads_per_process\n", argv[0]);
    return 0;
  }

  if (!parse_uint64(argv[1], "n", 0, UINT64_MAX, &config->limit) ||
      !parse_uint64(argv[2], "thread count", 1, MAX_THREAD_COUNT,
                    &thread_count)) {
    return 0;
  }

  config->thread_count = (int)thread_count;
  return 1;
}

static int parse_uint64(const char *text, const char *name, uint64_t minimum,
                        uint64_t maximum, uint64_t *value) {
  const char *cursor = text;
  char *end = NULL;
  uintmax_t parsed;

  while (isspace((unsigned char)*cursor)) {
    ++cursor;
  }
  if (*cursor == '-') {
    fprintf(stderr, "Error: %s must not be negative.\n", name);
    return 0;
  }

  errno = 0;
  parsed = strtoumax(cursor, &end, 10);
  if (cursor == end || errno == ERANGE || parsed > UINT64_MAX) {
    fprintf(stderr, "Error: %s is not a supported unsigned integer.\n", name);
    return 0;
  }

  while (isspace((unsigned char)*end)) {
    ++end;
  }
  if (*end != '\0') {
    fprintf(stderr, "Error: enter only one integer for %s.\n", name);
    return 0;
  }
  if ((uint64_t)parsed < minimum || (uint64_t)parsed > maximum) {
    fprintf(stderr, "Error: %s must be between %" PRIu64 " and %" PRIu64
                    ".\n",
            name, minimum, maximum);
    return 0;
  }

  *value = (uint64_t)parsed;
  return 1;
}

static size_t odd_candidate_count(uint64_t limit) {
  return (limit > 2) ? (size_t)((limit - 2) / 2) : 0;
}

static size_t cyclic_candidate_count(size_t odd_count, int rank,
                                     int process_count) {
  size_t rank_index = (size_t)rank;
  size_t process_stride = (size_t)process_count;

  if (rank_index >= odd_count) {
    return 0;
  }
  return 1 + (odd_count - 1 - rank_index) / process_stride;
}

static size_t global_odd_index(size_t local_index, int rank,
                               int process_count) {
  return (size_t)rank + local_index * (size_t)process_count;
}

static uint64_t odd_value(size_t global_index) {
  return UINT64_C(2) * (uint64_t)global_index + UINT64_C(3);
}

static int is_prime_by_trial_division(uint64_t candidate) {
  uint64_t divisor;
  uint64_t divisor_limit;

  if (candidate < 2) {
    return 0;
  }
  if (candidate == 2) {
    return 1;
  }
  if (candidate % 2 == 0) {
    return 0;
  }

  /* Same odd-only sqrt-bounded trial division used in the Week 4 code. */
  divisor_limit = (uint64_t)sqrt((double)candidate);
  for (divisor = 3; divisor <= divisor_limit; divisor += 2) {
    if (candidate % divisor == 0) {
      return 0;
    }
  }
  return 1;
}

static int compute_local_primes(int rank, int process_count, int thread_count,
                                LocalResult *result) {
  unsigned char *is_prime = result->is_prime;
  size_t local_count = result->candidate_count;
  uint64_t local_prime_count = 0;
  size_t local_index;

  if (local_count == 0) {
    result->prime_count = 0;
    return 1;
  }

#pragma omp parallel for default(none)                                        \
    shared(is_prime, local_count, rank, process_count)                        \
    num_threads(thread_count) schedule(dynamic, OMP_DYNAMIC_CHUNK)            \
    reduction(+ : local_prime_count)
  for (local_index = 0; local_index < local_count; ++local_index) {
    size_t global_index = global_odd_index(local_index, rank, process_count);
    uint64_t candidate = odd_value(global_index);
    int candidate_is_prime = is_prime_by_trial_division(candidate);

    is_prime[local_index] = (unsigned char)candidate_is_prime;
    local_prime_count += (uint64_t)candidate_is_prime;
  }

  result->prime_count = local_prime_count;
  return 1;
}

static int compact_local_primes(int rank, int process_count,
                                LocalResult *result) {
  size_t local_index;
  uint64_t output_index = 0;

  if (result->prime_count == 0) {
    return 1;
  }
  if (result->prime_count > SIZE_MAX / sizeof(*result->primes)) {
    return 0;
  }

  result->primes =
      malloc((size_t)result->prime_count * sizeof(*result->primes));
  if (result->primes == NULL) {
    return 0;
  }

  /* Sequential compaction preserves ascending order within this rank. */
  for (local_index = 0; local_index < result->candidate_count; ++local_index) {
    if (result->is_prime[local_index]) {
      size_t global_index = global_odd_index(local_index, rank, process_count);
      result->primes[output_index++] = odd_value(global_index);
    }
  }

  return output_index == result->prime_count;
}

static int merge_and_save_primes(const char *output_path, uint64_t limit,
                                 const uint64_t *gathered_primes,
                                 const int *receive_counts,
                                 const int *receive_displacements,
                                 int process_count,
                                 uint64_t expected_odd_prime_count) {
  FILE *output = NULL;
  char *output_buffer = NULL;
  MergeNode *heap = NULL;
  int heap_size = 0;
  uint64_t emitted = 0;
  int write_ok = 1;
  int process_index;

  heap = malloc((size_t)process_count * sizeof(*heap));
  if (heap == NULL && process_count > 0) {
    fputs("Error: unable to allocate the root merge heap.\n", stderr);
    return 0;
  }

  output = fopen(output_path, "w");
  if (output == NULL) {
    perror("Error opening prime output file");
    free(heap);
    return 0;
  }

  output_buffer = malloc(OUTPUT_BUFFER_SIZE);
  if (output_buffer != NULL) {
    (void)setvbuf(output, output_buffer, _IOFBF, OUTPUT_BUFFER_SIZE);
  }

  if (limit > 2 && fprintf(output, "2\n") < 0) {
    write_ok = 0;
  }

  for (process_index = 0; process_index < process_count; ++process_index) {
    if (receive_counts[process_index] > 0) {
      MergeNode node;

      node.rank = process_index;
      node.local_position = 0;
      node.value = gathered_primes[receive_displacements[process_index]];
      heap_push(heap, &heap_size, node);
    }
  }

  while (write_ok && heap_size > 0) {
    MergeNode node = heap_pop(heap, &heap_size);
    int next_position = node.local_position + 1;

    if (fprintf(output, "%" PRIu64 "\n", node.value) < 0) {
      write_ok = 0;
      break;
    }
    ++emitted;

    if (next_position < receive_counts[node.rank]) {
      MergeNode next_node;

      next_node.rank = node.rank;
      next_node.local_position = next_position;
      next_node.value = gathered_primes[receive_displacements[node.rank] +
                                          next_position];
      heap_push(heap, &heap_size, next_node);
    }
  }

  if (emitted != expected_odd_prime_count) {
    write_ok = 0;
  }
  if (fclose(output) == EOF) {
    write_ok = 0;
  }

  free(output_buffer);
  free(heap);

  if (!write_ok) {
    fputs("Error: failed while merging/writing the prime output file.\n",
          stderr);
  }
  return write_ok;
}

static void heap_push(MergeNode *heap, int *heap_size, MergeNode node) {
  int child = (*heap_size)++;

  heap[child] = node;
  while (child > 0) {
    int parent = (child - 1) / 2;

    if (!merge_node_less(heap[child], heap[parent])) {
      break;
    }
    {
      MergeNode temporary = heap[parent];
      heap[parent] = heap[child];
      heap[child] = temporary;
    }
    child = parent;
  }
}

static MergeNode heap_pop(MergeNode *heap, int *heap_size) {
  MergeNode minimum = heap[0];
  MergeNode last = heap[--(*heap_size)];
  int parent = 0;

  if (*heap_size == 0) {
    return minimum;
  }

  heap[0] = last;
  while (1) {
    int left_child = 2 * parent + 1;
    int right_child = left_child + 1;
    int smallest = parent;

    if (left_child < *heap_size &&
        merge_node_less(heap[left_child], heap[smallest])) {
      smallest = left_child;
    }
    if (right_child < *heap_size &&
        merge_node_less(heap[right_child], heap[smallest])) {
      smallest = right_child;
    }
    if (smallest == parent) {
      break;
    }

    {
      MergeNode temporary = heap[parent];
      heap[parent] = heap[smallest];
      heap[smallest] = temporary;
    }
    parent = smallest;
  }

  return minimum;
}

static int merge_node_less(MergeNode left, MergeNode right) {
  if (left.value != right.value) {
    return left.value < right.value;
  }
  return left.rank < right.rank;
}

static int all_processes_ok(int local_ok, MPI_Comm communicator) {
  int global_ok = 0;

  MPI_Allreduce(&local_ok, &global_ok, 1, MPI_INT, MPI_MIN, communicator);
  return global_ok;
}

static void free_local_result(LocalResult *result) {
  free(result->primes);
  free(result->is_prime);
  result->primes = NULL;
  result->is_prime = NULL;
  result->candidate_count = 0;
  result->prime_count = 0;
}
