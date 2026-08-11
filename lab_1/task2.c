/*
 * FIT3143 Lab 1 - Task 2: POSIX Threads prime search
 *
 * Purpose: Find every prime strictly less than a user-supplied limit using a
 *          parallel odd-only segmented Sieve of Eratosthenes.
 * Authors: Add team member names, student IDs, and Monash email addresses.
 * Build:   gcc -O3 -Wall -Wextra -pedantic task2.c -o task2 -pthread
 * Run:     ./task2
 */

#if !defined(_WIN32)
#define _POSIX_C_SOURCE 200809L
#endif

#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <pthread.h>
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
#define OUTPUT_FILE_BASENAME "task2_primes.txt"
#define OUTPUT_PATH_SIZE 4096

typedef struct {
  unsigned char *is_composite;
  size_t odd_count;
  size_t prime_count;
} SieveResult;

typedef struct {
  unsigned char *is_composite;
  const size_t *base_primes;
  size_t base_prime_count;
  size_t begin_index;
  size_t end_index;
} ThreadArguments;

static int read_size_value(const char *prompt, const char *name, size_t minimum,
                           size_t maximum, size_t *value);
static int monotonic_seconds(double *seconds);
static int run_serial_sieve(size_t limit, SieveResult *result);
static int run_parallel_sieve(size_t limit, size_t thread_count,
                              SieveResult *result);
static void *mark_segment(void *argument);
static int generate_base_primes(size_t limit, size_t **base_primes,
                                size_t *base_prime_count);
static size_t floor_square_root(size_t value);
static int sieve_results_equal(const SieveResult *left,
                               const SieveResult *right);
static int print_primes(size_t limit, const SieveResult *result);
static int build_output_path(const char *program_path, char *output_path,
                             size_t output_path_size);
static int save_primes(const char *output_path, const SieveResult *result);
static const char *last_path_separator(const char *path);
static size_t odd_value(size_t index);

int main(int argc, char *argv[]) {
  size_t limit;
  size_t thread_count;
  SieveResult serial_result = {NULL, 0, 0};
  SieveResult parallel_result = {NULL, 0, 0};
  char output_path[OUTPUT_PATH_SIZE];
  double serial_start;
  double serial_end;
  double parallel_start;
  double parallel_end;
  double serial_seconds;
  double parallel_seconds;
  double speedup;
  double efficiency;
  int output_ok;

  (void)argc;

  if (!read_size_value("Enter upper limit n: ", "n", 0, SIZE_MAX, &limit) ||
      !read_size_value("Enter number of threads: ", "thread count", 1,
                       MAX_THREAD_COUNT, &thread_count)) {
    return EXIT_FAILURE;
  }

  if (!monotonic_seconds(&serial_start)) {
    fputs("Error: unable to read the monotonic clock.\n", stderr);
    return EXIT_FAILURE;
  }
  if (!run_serial_sieve(limit, &serial_result)) {
    fprintf(stderr, "Error: unable to allocate the serial sieve below %zu.\n",
            limit);
    return EXIT_FAILURE;
  }
  if (!monotonic_seconds(&serial_end)) {
    fputs("Error: unable to read the monotonic clock.\n", stderr);
    free(serial_result.is_composite);
    return EXIT_FAILURE;
  }

  if (!monotonic_seconds(&parallel_start)) {
    fputs("Error: unable to read the monotonic clock.\n", stderr);
    free(serial_result.is_composite);
    return EXIT_FAILURE;
  }
  if (!run_parallel_sieve(limit, thread_count, &parallel_result)) {
    fputs("Error: the POSIX Threads prime search failed.\n", stderr);
    free(serial_result.is_composite);
    return EXIT_FAILURE;
  }
  if (!monotonic_seconds(&parallel_end)) {
    fputs("Error: unable to read the monotonic clock.\n", stderr);
    free(parallel_result.is_composite);
    free(serial_result.is_composite);
    return EXIT_FAILURE;
  }

  if (!sieve_results_equal(&serial_result, &parallel_result)) {
    fputs("Error: serial and POSIX Threads results do not match.\n", stderr);
    free(parallel_result.is_composite);
    free(serial_result.is_composite);
    return EXIT_FAILURE;
  }

  serial_seconds = serial_end - serial_start;
  parallel_seconds = parallel_end - parallel_start;
  speedup = (parallel_seconds > 0.0) ? serial_seconds / parallel_seconds : 0.0;
  efficiency = speedup / (double)thread_count;

  printf("Implementation: pthread\n");
  printf("Input limit: %zu\n", limit);
  printf("Thread count: %zu\n", thread_count);
  printf("Prime count: %zu\n", parallel_result.prime_count);
  printf("Serial computation time: %.9f seconds\n", serial_seconds);
  printf("Pthreads computation time: %.9f seconds\n", parallel_seconds);
  printf("Speedup: %.6f\n", speedup);
  printf("Efficiency: %.6f\n", efficiency);

  if (limit <= TERMINAL_OUTPUT_LIMIT) {
    output_ok = print_primes(limit, &parallel_result);
  } else {
    output_ok = build_output_path(argv[0], output_path, sizeof(output_path));
    if (!output_ok) {
      fputs("Error: unable to construct the prime output path.\n", stderr);
    } else {
      output_ok = save_primes(output_path, &parallel_result);
    }
    if (output_ok) {
      printf("Primes written to %s\n", output_path);
    }
  }

  free(parallel_result.is_composite);
  free(serial_result.is_composite);
  return output_ok ? EXIT_SUCCESS : EXIT_FAILURE;
}

/* Read one complete size_t value and reject negatives or trailing input. */
static int read_size_value(const char *prompt, const char *name, size_t minimum,
                           size_t maximum, size_t *value) {
  char buffer[INPUT_BUFFER_SIZE];
  char *cursor;
  char *end;
  uintmax_t parsed;

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

  cursor = buffer;
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

/* Use wall time because CPU time would accumulate work across all threads. */
static int monotonic_seconds(double *seconds) {
#if defined(_WIN32)
  LARGE_INTEGER frequency;
  LARGE_INTEGER counter;

  if (!QueryPerformanceFrequency(&frequency) ||
      !QueryPerformanceCounter(&counter)) {
    return 0;
  }
  *seconds = (double)counter.QuadPart / (double)frequency.QuadPart;
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

/* Task 1's odd-only sieve is repeated here as the speedup baseline. */
static int run_serial_sieve(size_t limit, SieveResult *result) {
  size_t index;

  result->odd_count = (limit > 2) ? (limit - 2) / 2 : 0;
  result->prime_count = (limit > 2) ? 1 : 0;
  result->is_composite = NULL;

  if (result->odd_count == 0) {
    return 1;
  }

  result->is_composite =
      calloc(result->odd_count, sizeof(*result->is_composite));
  if (result->is_composite == NULL) {
    return 0;
  }

  for (index = 0; index < result->odd_count; ++index) {
    size_t prime;
    size_t multiple_index;

    if (result->is_composite[index]) {
      continue;
    }

    prime = odd_value(index);
    if (prime > (limit - 1) / prime) {
      break;
    }

    multiple_index = (prime * prime - 3) / 2;
    for (; multiple_index < result->odd_count; multiple_index += prime) {
      result->is_composite[multiple_index] = 1;
    }
  }

  for (index = 0; index < result->odd_count; ++index) {
    if (!result->is_composite[index]) {
      ++result->prime_count;
    }
  }

  return 1;
}

/*
 * Threads own disjoint contiguous index ranges. For a segmented sieve this
 * is balanced, avoids locks, limits false sharing to segment boundaries, and
 * keeps every write race-free. Thread lifecycle overhead remains timed.
 */
static int run_parallel_sieve(size_t limit, size_t thread_count,
                              SieveResult *result) {
  pthread_t *threads = NULL;
  ThreadArguments *arguments = NULL;
  size_t *base_primes = NULL;
  size_t base_prime_count = 0;
  size_t base_segment_size;
  size_t extra_indices;
  size_t created_count = 0;
  size_t thread_index;
  size_t index;
  int operation_ok = 1;

  result->odd_count = (limit > 2) ? (limit - 2) / 2 : 0;
  result->prime_count = (limit > 2) ? 1 : 0;
  result->is_composite = NULL;

  if (result->odd_count == 0) {
    return 1;
  }

  result->is_composite =
      calloc(result->odd_count, sizeof(*result->is_composite));
  if (result->is_composite == NULL ||
      !generate_base_primes(limit, &base_primes, &base_prime_count)) {
    free(result->is_composite);
    result->is_composite = NULL;
    return 0;
  }

  if (thread_count > SIZE_MAX / sizeof(*threads) ||
      thread_count > SIZE_MAX / sizeof(*arguments)) {
    free(base_primes);
    free(result->is_composite);
    result->is_composite = NULL;
    return 0;
  }

  threads = malloc(thread_count * sizeof(*threads));
  arguments = malloc(thread_count * sizeof(*arguments));
  if (threads == NULL || arguments == NULL) {
    free(arguments);
    free(threads);
    free(base_primes);
    free(result->is_composite);
    result->is_composite = NULL;
    return 0;
  }

  base_segment_size = result->odd_count / thread_count;
  extra_indices = result->odd_count % thread_count;

  for (thread_index = 0; thread_index < thread_count; ++thread_index) {
    size_t begin_index = thread_index * base_segment_size;
    size_t segment_length = base_segment_size;
    int error_code;

    if (thread_index < extra_indices) {
      begin_index += thread_index;
      ++segment_length;
    } else {
      begin_index += extra_indices;
    }

    arguments[thread_index].is_composite = result->is_composite;
    arguments[thread_index].base_primes = base_primes;
    arguments[thread_index].base_prime_count = base_prime_count;
    arguments[thread_index].begin_index = begin_index;
    arguments[thread_index].end_index = begin_index + segment_length;

    error_code = pthread_create(&threads[thread_index], NULL, mark_segment,
                                &arguments[thread_index]);
    if (error_code != 0) {
      fprintf(stderr, "Error: pthread_create for thread %zu: %s\n",
              thread_index, strerror(error_code));
      operation_ok = 0;
      break;
    }
    ++created_count;
  }

  for (thread_index = 0; thread_index < created_count; ++thread_index) {
    int error_code = pthread_join(threads[thread_index], NULL);

    if (error_code != 0) {
      fprintf(stderr, "Error: pthread_join for thread %zu: %s\n", thread_index,
              strerror(error_code));
      operation_ok = 0;
    }
  }

  free(arguments);
  free(threads);
  free(base_primes);

  if (!operation_ok) {
    free(result->is_composite);
    result->is_composite = NULL;
    return 0;
  }

  for (index = 0; index < result->odd_count; ++index) {
    if (!result->is_composite[index]) {
      ++result->prime_count;
    }
  }

  return 1;
}

static void *mark_segment(void *argument) {
  ThreadArguments *thread = argument;
  size_t prime_index;

  for (prime_index = 0; prime_index < thread->base_prime_count; ++prime_index) {
    size_t prime = thread->base_primes[prime_index];
    size_t multiple_index = (prime * prime - 3) / 2;

    if (multiple_index < thread->begin_index) {
      size_t distance = thread->begin_index - multiple_index;
      size_t steps = distance / prime;

      if (distance % prime != 0) {
        ++steps;
      }
      multiple_index += steps * prime;
    }

    for (; multiple_index < thread->end_index; multiple_index += prime) {
      thread->is_composite[multiple_index] = 1;
    }
  }

  return NULL;
}

/* Generate the odd primes needed to mark composites in every segment. */
static int generate_base_primes(size_t limit, size_t **base_primes,
                                size_t *base_prime_count) {
  SieveResult base_result = {NULL, 0, 0};
  size_t root = floor_square_root(limit - 1);
  size_t index;
  size_t output_index = 0;

  *base_primes = NULL;
  *base_prime_count = 0;

  if (!run_serial_sieve(root + 1, &base_result)) {
    return 0;
  }

  if (base_result.prime_count > 0) {
    *base_prime_count = base_result.prime_count - 1;
  }
  if (*base_prime_count == 0) {
    free(base_result.is_composite);
    return 1;
  }
  if (*base_prime_count > SIZE_MAX / sizeof(**base_primes)) {
    free(base_result.is_composite);
    return 0;
  }

  *base_primes = malloc(*base_prime_count * sizeof(**base_primes));
  if (*base_primes == NULL) {
    free(base_result.is_composite);
    return 0;
  }

  for (index = 0; index < base_result.odd_count; ++index) {
    if (!base_result.is_composite[index]) {
      (*base_primes)[output_index++] = odd_value(index);
    }
  }

  free(base_result.is_composite);
  if (output_index != *base_prime_count) {
    free(*base_primes);
    *base_primes = NULL;
    *base_prime_count = 0;
    return 0;
  }
  return 1;
}

/* Integer binary search avoids floating-point rounding around perfect squares.
 */
static size_t floor_square_root(size_t value) {
  size_t low = 0;
  size_t high = (value < 4) ? value : value / 2 + 1;
  size_t result = 0;

  while (low <= high) {
    size_t middle = low + (high - low) / 2;

    if (middle == 0 || middle <= value / middle) {
      result = middle;
      low = middle + 1;
    } else {
      high = middle - 1;
    }
  }

  return result;
}

static int sieve_results_equal(const SieveResult *left,
                               const SieveResult *right) {
  if (left->odd_count != right->odd_count ||
      left->prime_count != right->prime_count) {
    return 0;
  }
  if (left->odd_count == 0) {
    return 1;
  }
  return memcmp(left->is_composite, right->is_composite,
                left->odd_count * sizeof(*left->is_composite)) == 0;
}

static int print_primes(size_t limit, const SieveResult *result) {
  size_t index;

  if (printf("Primes less than %zu:", limit) < 0) {
    return 0;
  }
  if (limit > 2 && printf(" 2") < 0) {
    return 0;
  }

  for (index = 0; index < result->odd_count; ++index) {
    if (!result->is_composite[index] && printf(" %zu", odd_value(index)) < 0) {
      return 0;
    }
  }

  return putchar('\n') != EOF;
}

static int build_output_path(const char *program_path, char *output_path,
                             size_t output_path_size) {
  const char *reference_path = __FILE__;
  const char *separator = last_path_separator(reference_path);
  size_t directory_length;
  size_t file_name_length = strlen(OUTPUT_FILE_BASENAME);

  if (separator == NULL) {
    reference_path = program_path;
    separator = last_path_separator(reference_path);
  }

  directory_length =
      (separator == NULL) ? 0 : (size_t)(separator - reference_path) + 1;
  if (directory_length + file_name_length + 1 > output_path_size) {
    return 0;
  }

  if (directory_length > 0) {
    memcpy(output_path, reference_path, directory_length);
  }
  memcpy(output_path + directory_length, OUTPUT_FILE_BASENAME,
         file_name_length + 1);
  return 1;
}

static int save_primes(const char *output_path, const SieveResult *result) {
  FILE *output = fopen(output_path, "w");
  size_t index;
  int write_ok = 1;

  if (output == NULL) {
    perror("Error opening prime output file");
    return 0;
  }

  if (result->prime_count > 0 && fprintf(output, "2\n") < 0) {
    write_ok = 0;
  }
  for (index = 0; write_ok && index < result->odd_count; ++index) {
    if (!result->is_composite[index] &&
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

static const char *last_path_separator(const char *path) {
  const char *forward_slash = strrchr(path, '/');
  const char *backslash = strrchr(path, '\\');

  if (forward_slash == NULL) {
    return backslash;
  }
  if (backslash == NULL) {
    return forward_slash;
  }
  return (forward_slash > backslash) ? forward_slash : backslash;
}

static size_t odd_value(size_t index) { return 2 * index + 3; }
