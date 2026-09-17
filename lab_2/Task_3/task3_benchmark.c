/*
 * FIT3143 Lab 1 - Task 3
 * OpenMP Prime Search - Lab 2 Benchmark Version
 *
 * This version preserves the original OpenMP implementation while adding
 * an overall wall-clock measurement suitable for comparison with Lab 2 MPI.
 *
 * Build:
 * gcc -std=c11 -O3 -Wall -Wextra -pedantic task3_benchmark.c \
 *     -o task3_benchmark -fopenmp -lm
 *
 * Run:
 * ./task3_benchmark
 *
 * Benchmark:
 * ./task3_benchmark --benchmark 10000000 4
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
static int read_size_value(const char *prompt, const char *name,
                           size_t minimum, size_t maximum, size_t *value);
static int parse_size_value(const char *text, const char *name,
                            size_t minimum, size_t maximum, size_t *value);
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
    double overall_start;
    double overall_end;

    double serial_seconds;
    double parallel_seconds;
    double overall_seconds;
    double speedup;

    int benchmark_mode;
    int output_ok = 1;

    if (!get_options(argc, argv, &limit, &thread_count, &benchmark_mode)) {
        return EXIT_FAILURE;
    }

    /*
     * Allocate and run the serial implementation first.
     *
     * This is used only to verify that the OpenMP result is correct and to
     * preserve the original Lab 1 speedup information. It is NOT included
     * in the OpenMP overall wall-clock measurement.
     */
    if (!allocate_result(limit, &serial_result)) {
        fprintf(stderr,
                "Error: unable to allocate serial prime flags below %zu.\n",
                limit);
        return EXIT_FAILURE;
    }

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

    /*
     * Start the OpenMP overall wall-clock measurement.
     *
     * This includes:
     * - parallel result allocation
     * - OpenMP computation
     * - output file writing
     *
     * It excludes:
     * - command-line parsing
     * - serial verification computation
     * - final console summary
     */
    if (!monotonic_seconds(&overall_start)) {
        fputs("Error: unable to read the monotonic clock.\n", stderr);
        output_ok = 0;
        goto cleanup;
    }

    if (!allocate_result(limit, &parallel_result)) {
        fprintf(stderr,
                "Error: unable to allocate OpenMP prime flags below %zu.\n",
                limit);
        output_ok = 0;
        goto cleanup;
    }

    omp_set_dynamic(0);

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

    /*
     * Benchmark mode always writes the output file so the OpenMP overall
     * wall-clock measurement is comparable with the MPI implementation.
     */
    if (benchmark_mode) {
        output_ok = save_primes(OUTPUT_FILE_BASENAME,
                                limit,
                                &parallel_result);
    } else if (limit < TERMINAL_OUTPUT_LIMIT) {
        output_ok = print_primes(limit, &parallel_result);
    } else {
        output_ok = save_primes(OUTPUT_FILE_BASENAME,
                                limit,
                                &parallel_result);
    }

    if (!output_ok) {
        goto cleanup;
    }

    if (!monotonic_seconds(&overall_end)) {
        fputs("Error: unable to read the monotonic clock.\n", stderr);
        output_ok = 0;
        goto cleanup;
    }

    /*
     * Correctness checking is deliberately outside the timed OpenMP region.
     * It is verification work rather than part of the parallel algorithm.
     */
    if (!results_equal(&serial_result, &parallel_result)) {
        fputs("Error: serial and OpenMP results do not match.\n", stderr);
        output_ok = 0;
        goto cleanup;
    }

    serial_seconds = serial_end - serial_start;
    parallel_seconds = parallel_end - parallel_start;
    overall_seconds = overall_end - overall_start;

    speedup = (parallel_seconds > 0.0)
                  ? serial_seconds / parallel_seconds
                  : 0.0;

    printf("\n");
    printf("Implementation: OpenMP\n");
    printf("Input limit: %zu\n", limit);
    printf("Thread count: %zu\n", thread_count);
    printf("Prime count: %zu\n", parallel_result.prime_count);
    printf("Serial computation time: %.9f seconds\n", serial_seconds);
    printf("OpenMP computation time: %.9f seconds\n", parallel_seconds);
    printf("Overall wall-clock time: %.9f seconds\n", overall_seconds);
    printf("Computation speedup: %.6f\n", speedup);
    printf("Computation efficiency: %.6f\n",
           speedup / (double)thread_count);

    if (benchmark_mode || limit >= TERMINAL_OUTPUT_LIMIT) {
        printf("Output file: %s\n", OUTPUT_FILE_BASENAME);
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

        return read_size_value("Enter upper limit n: ",
                               "n",
                               0,
                               SIZE_MAX,
                               limit) &&
               read_size_value("Enter number of threads: ",
                               "thread count",
                               1,
                               MAX_THREAD_COUNT,
                               thread_count);
    }

    if (argc == 4 && strcmp(argv[1], "--benchmark") == 0) {
        *benchmark_mode = 1;

        return parse_size_value(argv[2],
                                "n",
                                0,
                                SIZE_MAX,
                                limit) &&
               parse_size_value(argv[3],
                                "thread count",
                                1,
                                MAX_THREAD_COUNT,
                                thread_count);
    }

    fprintf(stderr,
            "Usage:\n"
            "  %s\n"
            "  %s --benchmark n threads\n",
            argv[0],
            argv[0]);

    return 0;
}

static int read_size_value(const char *prompt, const char *name,
                           size_t minimum, size_t maximum, size_t *value) {
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

    return parse_size_value(buffer,
                            name,
                            minimum,
                            maximum,
                            value);
}

static int parse_size_value(const char *text, const char *name,
                            size_t minimum, size_t maximum, size_t *value) {
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

    if (cursor == end ||
        errno == ERANGE ||
        parsed > (uintmax_t)SIZE_MAX) {
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

    if (parsed < (uintmax_t)minimum ||
        parsed > (uintmax_t)maximum) {
        fprintf(stderr,
                "Error: %s must be between %zu and %zu.\n",
                name,
                minimum,
                maximum);
        return 0;
    }

    *value = (size_t)parsed;
    return 1;
}

static int allocate_result(size_t limit, PrimeResult *result) {
    result->odd_count = (limit > 2)
                            ? (limit - 2) / 2
                            : 0;

    result->prime_count = 0;
    result->is_prime = NULL;

    if (result->odd_count == 0) {
        return 1;
    }

    result->is_prime =
        malloc(result->odd_count * sizeof(*result->is_prime));

    return result->is_prime != NULL;
}

static void search_serial(size_t limit, PrimeResult *result) {
    size_t prime_count = (limit > 2)
                             ? 1
                             : 0;

    for (size_t index = 0;
         index < result->odd_count;
         ++index) {
        int candidate_is_prime =
            is_prime_by_trial_division(odd_value(index));

        result->is_prime[index] =
            (unsigned char)candidate_is_prime;

        prime_count +=
            (size_t)candidate_is_prime;
    }

    result->prime_count = prime_count;
}

/*
 * Static contiguous range ownership.
 *
 * Each OpenMP worker receives a disjoint range of odd candidates.
 * The reduction safely combines each thread's contribution to prime_count.
 */
static int search_openmp(size_t limit, size_t thread_count,
                         PrimeResult *result) {
    unsigned char *is_prime = result->is_prime;
    size_t odd_count = result->odd_count;
    size_t partition_count = thread_count;
    size_t prime_count = (limit > 2)
                             ? 1
                             : 0;

#pragma omp parallel for default(none)                                      \
    shared(is_prime, odd_count, partition_count) num_threads(thread_count)  \
    schedule(static) reduction(+ : prime_count)
    for (size_t partition_index = 0;
         partition_index < partition_count;
         ++partition_index) {
        size_t base_block =
            odd_count / partition_count;

        size_t extra =
            odd_count % partition_count;

        size_t begin =
            partition_index * base_block +
            ((partition_index < extra)
                 ? partition_index
                 : extra);

        size_t end =
            begin +
            base_block +
            ((partition_index < extra)
                 ? 1
                 : 0);

        for (size_t index = begin;
             index < end;
             ++index) {
            int candidate_is_prime =
                is_prime_by_trial_division(
                    odd_value(index));

            is_prime[index] =
                (unsigned char)candidate_is_prime;

            prime_count +=
                (size_t)candidate_is_prime;
        }
    }

    result->prime_count = prime_count;

    return 1;
}

static int is_prime_by_trial_division(size_t candidate) {
    if (candidate < 2) {
        return 0;
    }

    if (candidate == 2) {
        return 1;
    }

    if (candidate % 2 == 0) {
        return 0;
    }

    size_t divisor_limit =
        (size_t)sqrt((double)candidate);

    for (size_t divisor = 3;
         divisor <= divisor_limit;
         divisor += 2) {
        if (candidate % divisor == 0) {
            return 0;
        }
    }

    return 1;
}

static int monotonic_seconds(double *seconds) {
#if defined(_WIN32)
    LARGE_INTEGER frequency;
    LARGE_INTEGER timestamp;

    if (!QueryPerformanceFrequency(&frequency) ||
        !QueryPerformanceCounter(&timestamp)) {
        return 0;
    }

    *seconds =
        (double)timestamp.QuadPart /
        (double)frequency.QuadPart;
#else
    struct timespec timestamp;

    if (clock_gettime(CLOCK_MONOTONIC,
                      &timestamp) != 0) {
        return 0;
    }

    *seconds =
        (double)timestamp.tv_sec +
        (double)timestamp.tv_nsec /
            1000000000.0;
#endif

    return 1;
}

static int results_equal(const PrimeResult *left,
                         const PrimeResult *right) {
    if (left->odd_count != right->odd_count ||
        left->prime_count != right->prime_count) {
        return 0;
    }

    return left->odd_count == 0 ||
           memcmp(left->is_prime,
                  right->is_prime,
                  left->odd_count *
                      sizeof(*left->is_prime)) == 0;
}

static int print_primes(size_t limit,
                        const PrimeResult *result) {
    if (printf("Primes less than %zu:",
               limit) < 0) {
        return 0;
    }

    if (limit > 2 &&
        printf(" 2") < 0) {
        return 0;
    }

    for (size_t index = 0;
         index < result->odd_count;
         ++index) {
        if (result->is_prime[index] &&
            printf(" %zu",
                   odd_value(index)) < 0) {
            return 0;
        }
    }

    return putchar('\n') != EOF;
}

static int save_primes(const char *output_path,
                       size_t limit,
                       const PrimeResult *result) {
    FILE *output =
        fopen(output_path, "w");

    int write_ok = 1;

    if (output == NULL) {
        perror("Error opening prime output file");
        return 0;
    }

    if (limit > 2 &&
        fprintf(output, "2\n") < 0) {
        write_ok = 0;
    }

    for (size_t index = 0;
         write_ok &&
         index < result->odd_count;
         ++index) {
        if (result->is_prime[index] &&
            fprintf(output,
                    "%zu\n",
                    odd_value(index)) < 0) {
            write_ok = 0;
        }
    }

    if (fclose(output) == EOF) {
        write_ok = 0;
    }

    if (!write_ok) {
        fputs("Error: failed while writing the prime output file.\n",
              stderr);
    }

    return write_ok;
}

static size_t odd_value(size_t index) {
    return 2 * index + 3;
}
