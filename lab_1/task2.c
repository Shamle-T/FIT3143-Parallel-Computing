/*
 * FIT3143 Lab 1 - Task 2: POSIX Threads prime search
 *
 * Finds every prime strictly below n using a range-partitioned Pthreads
 * Sieve of Eratosthenes.
 * Add both team members' names, student IDs, and Monash email addresses.
 *
 * Build: gcc -std=c11 -O3 -Wall -Wextra -pedantic task2.c -o task2 -pthread
 * Run:   ./task2
 *        ./task2 --benchmark 10000000 4
 */

#define _POSIX_C_SOURCE 200809L

#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define INPUT_BUFFER_SIZE 128
#define MAX_THREAD_COUNT 1024
#define TERMINAL_OUTPUT_LIMIT 100
#define OUTPUT_FILE_BASENAME "task2_primes.txt"

typedef struct {
    unsigned char *is_prime;
    size_t odd_count;
    size_t prime_count;
} PrimeResult;

typedef struct {
    unsigned char *is_prime;
    const PrimeResult *base_primes;
    size_t limit;
    size_t begin_index;
    size_t end_index;
    size_t prime_count;
} ThreadArguments;

static int get_options(int argc, char *argv[], size_t *limit,
                       size_t *thread_count, int *benchmark_mode);
static int read_size_value(const char *prompt, const char *name,
                           size_t minimum, size_t maximum, size_t *value);
static int parse_size_value(const char *text, const char *name,
                            size_t minimum, size_t maximum, size_t *value);
static int allocate_result(size_t limit, PrimeResult *result);
static int search_serial(size_t limit, PrimeResult *result);
static void search_base_sieve(size_t limit, PrimeResult *result);
static int search_pthreads(pthread_t *threads, ThreadArguments *arguments,
                           size_t thread_count, size_t limit,
                           PrimeResult *result);
static void *search_thread(void *argument);
static void mark_sieve_range(size_t limit, const PrimeResult *base_primes,
                             unsigned char *is_prime, size_t begin,
                             size_t end);
static size_t sieve_base_limit(size_t limit);
static int monotonic_seconds(double *seconds);
static int results_equal(const PrimeResult *left, const PrimeResult *right);
static int print_primes(size_t limit, const PrimeResult *result);
static int save_primes(const char *output_path, size_t limit,
                       const PrimeResult *result);
static size_t odd_index(size_t value);
static size_t odd_value(size_t index);

int main(int argc, char *argv[])
{
    PrimeResult serial_result = {NULL, 0, 0};
    PrimeResult parallel_result = {NULL, 0, 0};
    pthread_t *threads = NULL;
    ThreadArguments *arguments = NULL;
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
        fprintf(stderr,
                "Error: unable to allocate the prime flags below %zu.\n",
                limit);
        free(parallel_result.is_prime);
        free(serial_result.is_prime);
        return EXIT_FAILURE;
    }

    threads = malloc(thread_count * sizeof(*threads));
    arguments = malloc(thread_count * sizeof(*arguments));
    if (threads == NULL || arguments == NULL) {
        fputs("Error: unable to allocate Pthreads metadata.\n", stderr);
        free(arguments);
        free(threads);
        free(parallel_result.is_prime);
        free(serial_result.is_prime);
        return EXIT_FAILURE;
    }

    if (!monotonic_seconds(&serial_start)) {
        fputs("Error: unable to read the monotonic clock.\n", stderr);
        output_ok = 0;
        goto cleanup;
    }
    if (!search_serial(limit, &serial_result)) {
        output_ok = 0;
        goto cleanup;
    }
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
    if (!search_pthreads(threads, arguments, thread_count, limit,
                         &parallel_result)) {
        output_ok = 0;
        goto cleanup;
    }
    if (!monotonic_seconds(&parallel_end)) {
        fputs("Error: unable to read the monotonic clock.\n", stderr);
        output_ok = 0;
        goto cleanup;
    }

    if (!results_equal(&serial_result, &parallel_result)) {
        fputs("Error: serial and Pthreads results do not match.\n", stderr);
        output_ok = 0;
        goto cleanup;
    }

    serial_seconds = serial_end - serial_start;
    parallel_seconds = parallel_end - parallel_start;
    speedup = (parallel_seconds > 0.0)
                  ? serial_seconds / parallel_seconds
                  : 0.0;

    printf("Implementation: pthread\n");
    printf("Input limit: %zu\n", limit);
    printf("Thread count: %zu\n", thread_count);
    printf("Prime count: %zu\n", parallel_result.prime_count);
    printf("Serial computation time: %.9f seconds\n", serial_seconds);
    printf("Pthreads computation time: %.9f seconds\n", parallel_seconds);
    printf("Speedup: %.6f\n", speedup);
    printf("Efficiency: %.6f\n", speedup / (double)thread_count);

    if (!benchmark_mode) {
        if (limit < TERMINAL_OUTPUT_LIMIT) {
            output_ok = print_primes(limit, &parallel_result);
        } else {
            output_ok = save_primes(OUTPUT_FILE_BASENAME, limit,
                                    &parallel_result);
            if (output_ok) {
                printf("Primes written to %s\n", OUTPUT_FILE_BASENAME);
            }
        }
    }

cleanup:
    free(arguments);
    free(threads);
    free(parallel_result.is_prime);
    free(serial_result.is_prime);
    return output_ok ? EXIT_SUCCESS : EXIT_FAILURE;
}

static int get_options(int argc, char *argv[], size_t *limit,
                       size_t *thread_count, int *benchmark_mode)
{
    if (argc == 1) {
        *benchmark_mode = 0;
        return read_size_value("Enter upper limit n: ", "n", 0, SIZE_MAX,
                               limit) &&
               read_size_value("Enter number of threads: ", "thread count",
                               1, MAX_THREAD_COUNT, thread_count);
    }
    if (argc == 4 && strcmp(argv[1], "--benchmark") == 0) {
        *benchmark_mode = 1;
        return parse_size_value(argv[2], "n", 0, SIZE_MAX, limit) &&
               parse_size_value(argv[3], "thread count", 1,
                                MAX_THREAD_COUNT, thread_count);
    }

    fprintf(stderr, "Usage: %s [--benchmark n threads]\n", argv[0]);
    return 0;
}

static int read_size_value(const char *prompt, const char *name,
                           size_t minimum, size_t maximum, size_t *value)
{
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

static int parse_size_value(const char *text, const char *name,
                            size_t minimum, size_t maximum, size_t *value)
{
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
        fprintf(stderr, "Error: %s must be between %zu and %zu.\n", name,
                minimum, maximum);
        return 0;
    }

    *value = (size_t)parsed;
    return 1;
}

static int allocate_result(size_t limit, PrimeResult *result)
{
    result->odd_count = (limit > 2) ? (limit - 2) / 2 : 0;
    result->prime_count = 0;
    result->is_prime = NULL;

    if (result->odd_count == 0) {
        return 1;
    }
    result->is_prime = malloc(result->odd_count * sizeof(*result->is_prime));
    return result->is_prime != NULL;
}

static int search_serial(size_t limit, PrimeResult *result)
{
    PrimeResult base_result = {NULL, 0, 0};
    size_t base_limit = sieve_base_limit(limit);
    size_t index;
    size_t prime_count = (limit > 2) ? 1 : 0;

    if (!allocate_result(base_limit, &base_result)) {
        fputs("Error: unable to allocate base prime flags.\n", stderr);
        return 0;
    }
    search_base_sieve(base_limit, &base_result);

    if (result->odd_count > 0) {
        memset(result->is_prime, 1, result->odd_count);
    }
    mark_sieve_range(limit, &base_result, result->is_prime, 0,
                     result->odd_count);
    for (index = 0; index < result->odd_count; ++index) {
        prime_count += result->is_prime[index];
    }

    result->prime_count = prime_count;
    free(base_result.is_prime);
    return 1;
}

static void search_base_sieve(size_t limit, PrimeResult *result)
{
    size_t index;
    size_t prime_count = (limit > 2) ? 1 : 0;

    if (result->odd_count > 0) {
        memset(result->is_prime, 1, result->odd_count);
    }
    for (index = 0; index < result->odd_count; ++index) {
        size_t prime;
        size_t composite;

        if (!result->is_prime[index]) {
            continue;
        }
        ++prime_count;

        prime = odd_value(index);
        if (prime > (limit - 1) / prime) {
            continue;
        }
        for (composite = prime * prime; composite < limit;
             composite += 2 * prime) {
            result->is_prime[odd_index(composite)] = 0;
        }
    }
    result->prime_count = prime_count;
}

static int search_pthreads(pthread_t *threads, ThreadArguments *arguments,
                           size_t thread_count, size_t limit,
                           PrimeResult *result)
{
    PrimeResult base_result = {NULL, 0, 0};
    size_t base_limit = sieve_base_limit(limit);
    size_t created_count = 0;
    size_t thread_index;
    size_t prime_count = (limit > 2) ? 1 : 0;
    int operation_ok = 1;

    if (!allocate_result(base_limit, &base_result)) {
        fputs("Error: unable to allocate base prime flags.\n", stderr);
        return 0;
    }
    search_base_sieve(base_limit, &base_result);

    if (result->odd_count > 0) {
        memset(result->is_prime, 1, result->odd_count);
    }
    for (thread_index = 0; thread_index < thread_count; ++thread_index) {
        size_t base_block = result->odd_count / thread_count;
        size_t extra = result->odd_count % thread_count;
        size_t begin = thread_index * base_block +
                       ((thread_index < extra) ? thread_index : extra);
        int error_code;

        arguments[thread_index].is_prime = result->is_prime;
        arguments[thread_index].base_primes = &base_result;
        arguments[thread_index].limit = limit;
        arguments[thread_index].begin_index = begin;
        arguments[thread_index].end_index =
            begin + base_block + ((thread_index < extra) ? 1 : 0);
        arguments[thread_index].prime_count = 0;

        error_code = pthread_create(&threads[thread_index], NULL,
                                    search_thread, &arguments[thread_index]);
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
            fprintf(stderr, "Error: pthread_join for thread %zu: %s\n",
                    thread_index, strerror(error_code));
            operation_ok = 0;
        } else {
            prime_count += arguments[thread_index].prime_count;
        }
    }

    if (operation_ok) {
        result->prime_count = prime_count;
    }
    free(base_result.is_prime);
    return operation_ok;
}

/*
 * Each thread owns one contiguous slice of the odd-number flag array. Sieve
 * marking stays race-free because no two threads write the same candidate
 * index, and the final count is collected from thread-local totals.
 */
static void *search_thread(void *argument)
{
    ThreadArguments *thread = argument;
    size_t prime_count = 0;
    size_t index;

    mark_sieve_range(thread->limit, thread->base_primes, thread->is_prime,
                     thread->begin_index, thread->end_index);
    for (index = thread->begin_index; index < thread->end_index; ++index) {
        prime_count += thread->is_prime[index];
    }
    thread->prime_count = prime_count;
    return NULL;
}

static void mark_sieve_range(size_t limit, const PrimeResult *base_primes,
                             unsigned char *is_prime, size_t begin,
                             size_t end)
{
    size_t base_index;
    size_t segment_start;

    if (begin >= end) {
        return;
    }
    segment_start = odd_value(begin);

    for (base_index = 0; base_index < base_primes->odd_count; ++base_index) {
        size_t prime;
        size_t start_value;
        size_t remainder;
        size_t index;

        if (!base_primes->is_prime[base_index]) {
            continue;
        }
        prime = odd_value(base_index);
        if (prime > (limit - 1) / prime) {
            break;
        }

        start_value = prime * prime;
        if (start_value < segment_start) {
            remainder = segment_start % prime;
            start_value = (remainder == 0)
                              ? segment_start
                              : segment_start + prime - remainder;
            if (start_value % 2 == 0) {
                start_value += prime;
            }
        }

        for (index = odd_index(start_value); index < end; index += prime) {
            is_prime[index] = 0;
        }
    }
}

static size_t sieve_base_limit(size_t limit)
{
    size_t factor = 1;

    if (limit == 0) {
        return 0;
    }
    while (factor <= (limit - 1) / factor) {
        ++factor;
    }
    return factor;
}

/* Wall time is used consistently so parallel CPU time is not accumulated. */
static int monotonic_seconds(double *seconds)
{
    clock_t timestamp = clock();

    if (timestamp == (clock_t)-1) {
        return 0;
    }
    *seconds = (double)timestamp / CLOCKS_PER_SEC;
    return 1;
}

static int results_equal(const PrimeResult *left, const PrimeResult *right)
{
    if (left->odd_count != right->odd_count ||
        left->prime_count != right->prime_count) {
        return 0;
    }
    return left->odd_count == 0 ||
           memcmp(left->is_prime, right->is_prime,
                  left->odd_count * sizeof(*left->is_prime)) == 0;
}

static int print_primes(size_t limit, const PrimeResult *result)
{
    size_t index;

    if (printf("Primes less than %zu:", limit) < 0) {
        return 0;
    }
    if (limit > 2 && printf(" 2") < 0) {
        return 0;
    }
    for (index = 0; index < result->odd_count; ++index) {
        if (result->is_prime[index] &&
            printf(" %zu", odd_value(index)) < 0) {
            return 0;
        }
    }
    return putchar('\n') != EOF;
}

static int save_primes(const char *output_path, size_t limit,
                       const PrimeResult *result)
{
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

static size_t odd_index(size_t value)
{
    return (value - 3) / 2;
}

static size_t odd_value(size_t index)
{
    return 2 * index + 3;
}
