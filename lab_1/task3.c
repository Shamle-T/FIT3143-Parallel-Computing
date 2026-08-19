/*
 * FIT3143 Lab 1 - Task 3: OpenMP prime search
 *
 * Finds every prime strictly below n using odd-only trial division. Static
 * partitions give each OpenMP worker a disjoint contiguous candidate range.
 *
 * Team members:
 * - Savin Vindiv De Alwis, 35221631, sdea0018@student.monash.edu
 * - Willwara Arachchilage Shamle Imal Thilaksiri, 35512075,
 *   wthi0003@student.monash.edu
 *
 * Build: gcc -std=c11 -O3 -Wall -Wextra -pedantic task3.c -o task3 -fopenmp -lm
 * Run:   ./task3
 *        ./task3 --benchmark 10000000 4
 */

#define _POSIX_C_SOURCE 200809L

#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <math.h>
#include <omp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if defined(_WIN32)
#include <windows.h>
#endif

#define INPUT_BUFFER_SIZE 128
#define MAX_THREAD_COUNT 1024
#define TERMINAL_OUTPUT_LIMIT 100
#define OUTPUT_FILE_BASENAME "task3_primes.txt"

typedef struct {
  unsigned char *is_prime;
  size_t odd_count;
  size_t prime_count;
} PrimeResult;

static int get_options(int argc, char *argv[], size_t *limit,
                       size_t *thread_count, int *benchmark_mode);
static int read_size_value(const char *prompt, const char *name, size_t minimum,
                           size_t maximum, size_t *value);
static int parse_size_value(const char *text, const char *name, size_t minimum,
                            size_t maximum, size_t *value);
static int allocate_result(size_t limit, PrimeResult *result);
static void search_serial(size_t limit, PrimeResult *result);
static int search_openmp(size_t limit, size_t thread_count,
                         PrimeResult *result);
static int is_prime_by_trial_division(size_t candidate);
static int monotonic_seconds(double *seconds);
static int results_equal(const PrimeResult *left, const PrimeResult *right);
static int print_primes(size_t limit, const PrimeResult *result);
static int save_primes(const char *output_path, size_t limit,
                       const PrimeResult *result);
static size_t odd_value(size_t index);

int main(int argc, char *argv[]) {
  PrimeResult serial_result = {NULL, 0, 0};
  PrimeResult parallel_result = {NULL, 0, 0};
  size_t limit;
  size_t thread_count;
  double serial_start;
  double serial_end;
  double parallel_start;
  double parallel_end;
  double serial_seconds;
  double parallel_seconds;
  double speedup;
  int benchmark_mode;
  int output_ok = 1;

  if (!get_options(argc, argv, &limit, &thread_count, &benchmark_mode)) {
    return EXIT_FAILURE;
  }
  if (!allocate_result(limit, &serial_result) ||
      !allocate_result(limit, &parallel_result)) {
    fprintf(stderr, "Error: unable to allocate the prime flags below %zu.\n",
            limit);
    free(parallel_result.is_prime);
    free(serial_result.is_prime);
    return EXIT_FAILURE;
  }

  omp_set_dynamic(0);

  if (!monotonic_seconds(&serial_start)) {
    fputs("Error: unable to read the monotonic clock.\n", stderr);
    output_ok = 0;
    goto cleanup;
  }
  search_serial(limit, &serial_result);
  if (!monotonic_seconds(&serial_end)) {
    fputs("Error: unable to read the monotonic clock.\n", stderr);
    output_ok = 0;
    goto cleanup;
  }

  if (!monotonic_seconds(&parallel_start)) {
    fputs("Error: unable to read the monotonic clock.\n", stderr);
    output_ok = 0;
    goto cleanup;
  }
  if (!search_openmp(limit, thread_count, &parallel_result)) {
    output_ok = 0;
    goto cleanup;
  }
  if (!monotonic_seconds(&parallel_end)) {
    fputs("Error: unable to read the monotonic clock.\n", stderr);
    output_ok = 0;
    goto cleanup;
  }

  if (!results_equal(&serial_result, &parallel_result)) {
    fputs("Error: serial and OpenMP results do not match.\n", stderr);
    output_ok = 0;
    goto cleanup;
  }

  serial_seconds = serial_end - serial_start;
  parallel_seconds = parallel_end - parallel_start;
  speedup = (parallel_seconds > 0.0) ? serial_seconds / parallel_seconds : 0.0;

  printf("Implementation: openmp\n");
  printf("Input limit: %zu\n", limit);
  printf("Thread count: %zu\n", thread_count);
  printf("Prime count: %zu\n", parallel_result.prime_count);
  printf("Serial computation time: %.9f seconds\n", serial_seconds);
  printf("OpenMP computation time: %.9f seconds\n", parallel_seconds);
  printf("Speedup: %.6f\n", speedup);
  printf("Efficiency: %.6f\n", speedup / (double)thread_count);

  if (!benchmark_mode) {
    if (limit < TERMINAL_OUTPUT_LIMIT) {
      output_ok = print_primes(limit, &parallel_result);
    } else {
      output_ok = save_primes(OUTPUT_FILE_BASENAME, limit, &parallel_result);
      if (output_ok) {
        printf("Primes written to %s\n", OUTPUT_FILE_BASENAME);
      }
    }
  }

cleanup:
  free(parallel_result.is_prime);
  free(serial_result.is_prime);
  return output_ok ? EXIT_SUCCESS : EXIT_FAILURE;
}

static int get_options(int argc, char *argv[], size_t *limit,
                       size_t *thread_count, int *benchmark_mode) {
  if (argc == 1) {
    *benchmark_mode = 0;
    return read_size_value("Enter upper limit n: ", "n", 0, SIZE_MAX, limit) &&
           read_size_value("Enter number of threads: ", "thread count", 1,
                           MAX_THREAD_COUNT, thread_count);
  }
  if (argc == 4 && strcmp(argv[1], "--benchmark") == 0) {
    *benchmark_mode = 1;
    return parse_size_value(argv[2], "n", 0, SIZE_MAX, limit) &&
           parse_size_value(argv[3], "thread count", 1, MAX_THREAD_COUNT,
                            thread_count);
  }

  fprintf(stderr, "Usage: %s [--benchmark n threads]\n", argv[0]);
  return 0;
}

static int read_size_value(const char *prompt, const char *name, size_t minimum,
                           size_t maximum, size_t *value) {
  char buffer[INPUT_BUFFER_SIZE];

  fputs(prompt, stdout);
  fflush(stdout);
  if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
    fprintf(stderr, "Error: failed to read %s.\n", name);
    return 0;
  }
  if (strchr(buffer, '\n') == NULL && !feof(stdin)) {
    fprintf(stderr, "Error: %s input is too long.\n", name);
    return 0;
  }
  return parse_size_value(buffer, name, minimum, maximum, value);
}

static int parse_size_value(const char *text, const char *name, size_t minimum,
                            size_t maximum, size_t *value) {
  const char *cursor = text;
  char *end;
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
  if (cursor == end || errno == ERANGE || parsed > (uintmax_t)SIZE_MAX) {
    fprintf(stderr, "Error: %s is not a supported integer.\n", name);
    return 0;
  }
  while (isspace((unsigned char)*end)) {
    ++end;
  }
  if (*end != '\0') {
    fprintf(stderr, "Error: enter only one integer for %s.\n", name);
    return 0;
  }
  if (parsed < (uintmax_t)minimum || parsed > (uintmax_t)maximum) {
    fprintf(stderr, "Error: %s must be between %zu and %zu.\n", name, minimum,
            maximum);
    return 0;
  }

  *value = (size_t)parsed;
  return 1;
}

static int allocate_result(size_t limit, PrimeResult *result) {
  result->odd_count = (limit > 2) ? (limit - 2) / 2 : 0;
  result->prime_count = 0;
  result->is_prime = NULL;

  if (result->odd_count == 0) {
    return 1;
  }
  result->is_prime = malloc(result->odd_count * sizeof(*result->is_prime));
  return result->is_prime != NULL;
}

static void search_serial(size_t limit, PrimeResult *result) {
  size_t index;
  size_t prime_count = (limit > 2) ? 1 : 0;

  for (index = 0; index < result->odd_count; ++index) {
    int is_prime = is_prime_by_trial_division(odd_value(index));

    result->is_prime[index] = (unsigned char)is_prime;
    prime_count += (size_t)is_prime;
  }
  result->prime_count = prime_count;
}

/*
 * Static range ownership keeps writes race-free: every iteration processes a
 * distinct slice of odd candidates. The reduction combines local counts
 * without a critical section.
 */
static int search_openmp(size_t limit, size_t thread_count,
                         PrimeResult *result) {
  unsigned char *is_prime = result->is_prime;
  size_t odd_count = result->odd_count;
  size_t partition_count = thread_count;
  size_t prime_count = (limit > 2) ? 1 : 0;
  size_t partition_index;

#pragma omp parallel for default(none)                                         \
    shared(is_prime, odd_count, partition_count) num_threads(thread_count)     \
    schedule(static) reduction(+ : prime_count)
  for (partition_index = 0; partition_index < partition_count;
       ++partition_index) {
    size_t base_block = odd_count / partition_count;
    size_t extra = odd_count % partition_count;
    size_t begin = partition_index * base_block +
                   ((partition_index < extra) ? partition_index : extra);
    size_t end = begin + base_block + ((partition_index < extra) ? 1 : 0);
    size_t index;

    for (index = begin; index < end; ++index) {
      int candidate_is_prime = is_prime_by_trial_division(odd_value(index));

      is_prime[index] = (unsigned char)candidate_is_prime;
      prime_count += (size_t)candidate_is_prime;
    }
  }
  result->prime_count = prime_count;
  return 1;
}

static int is_prime_by_trial_division(size_t candidate) {
  size_t divisor;
  size_t divisor_limit;

  if (candidate < 2) {
    return 0;
  }
  if (candidate == 2) {
    return 1;
  }
  if (candidate % 2 == 0) {
    return 0;
  }

  divisor_limit = (size_t)sqrt((double)candidate);
  for (divisor = 3; divisor <= divisor_limit; divisor += 2) {
    if (candidate % divisor == 0) {
      return 0;
    }
  }
  return 1;
}

/* Monotonic wall time measures elapsed time without accumulating CPU time. */
static int monotonic_seconds(double *seconds) {
#if defined(_WIN32)
  LARGE_INTEGER frequency;
  LARGE_INTEGER timestamp;

  if (!QueryPerformanceFrequency(&frequency) ||
      !QueryPerformanceCounter(&timestamp)) {
    return 0;
  }
  *seconds = (double)timestamp.QuadPart / (double)frequency.QuadPart;
#else
  struct timespec timestamp;

  if (clock_gettime(CLOCK_MONOTONIC, &timestamp) != 0) {
    return 0;
  }
  *seconds =
      (double)timestamp.tv_sec + (double)timestamp.tv_nsec / 1000000000.0;
#endif
  return 1;
}

static int results_equal(const PrimeResult *left, const PrimeResult *right) {
  if (left->odd_count != right->odd_count ||
      left->prime_count != right->prime_count) {
    return 0;
  }
  return left->odd_count == 0 ||
         memcmp(left->is_prime, right->is_prime,
                left->odd_count * sizeof(*left->is_prime)) == 0;
}

static int print_primes(size_t limit, const PrimeResult *result) {
  size_t index;

  if (printf("Primes less than %zu:", limit) < 0) {
    return 0;
  }
  if (limit > 2 && printf(" 2") < 0) {
    return 0;
  }
  for (index = 0; index < result->odd_count; ++index) {
    if (result->is_prime[index] && printf(" %zu", odd_value(index)) < 0) {
      return 0;
    }
  }
  return putchar('\n') != EOF;
}

static int save_primes(const char *output_path, size_t limit,
                       const PrimeResult *result) {
  FILE *output = fopen(output_path, "w");
  size_t index;
  int write_ok = 1;

  if (output == NULL) {
    perror("Error opening prime output file");
    return 0;
  }
  if (limit > 2 && fprintf(output, "2\n") < 0) {
    write_ok = 0;
  }
  for (index = 0; write_ok && index < result->odd_count; ++index) {
    if (result->is_prime[index] &&
        fprintf(output, "%zu\n", odd_value(index)) < 0) {
      write_ok = 0;
    }
  }
  if (fclose(output) == EOF) {
    write_ok = 0;
  }
  if (!write_ok) {
    fputs("Error: failed while writing the prime output file.\n", stderr);
  }
  return write_ok;
}

static size_t odd_value(size_t index) { return 2 * index + 3; }
