/*
 * FIT3143 Lab 1 - Task 1: Serial prime search
 *
 * Purpose: Find every prime strictly less than a user-supplied limit using an
 *          odd-only Sieve of Eratosthenes.
 * Authors: Add team member names, student IDs, and Monash email addresses.
 * Build:   gcc -O3 -Wall -Wextra -pedantic task1.c -o task1
 * Run:     ./task1
 */

#if !defined(_WIN32)
#define _POSIX_C_SOURCE 200809L
#endif

#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if defined(_WIN32)
#include <windows.h>
#endif

#define INPUT_BUFFER_SIZE 128
#define TERMINAL_OUTPUT_LIMIT 100
#define OUTPUT_FILE_BASENAME "task1_primes.txt"
#define OUTPUT_PATH_SIZE 4096

typedef struct {
    unsigned char *is_composite;
    size_t odd_count;
    size_t prime_count;
} SieveResult;

static int read_limit(size_t *limit);
static int monotonic_seconds(double *seconds);
static int run_odd_sieve(size_t limit, SieveResult *result);
static int print_primes(size_t limit, const SieveResult *result);
static int build_output_path(const char *program_path, char *output_path,
                             size_t output_path_size);
static int save_primes(const char *output_path, const SieveResult *result);
static const char *last_path_separator(const char *path);
static size_t odd_value(size_t index);

int main(int argc, char *argv[])
{
    size_t limit;
    SieveResult result = {NULL, 0, 0};
    char output_path[OUTPUT_PATH_SIZE];
    double start_time;
    double end_time;
    int output_ok;

    (void)argc;

    if (!read_limit(&limit)) {
        return EXIT_FAILURE;
    }

    if (!monotonic_seconds(&start_time)) {
        fputs("Error: unable to read the monotonic clock.\n", stderr);
        return EXIT_FAILURE;
    }

    if (!run_odd_sieve(limit, &result)) {
        fprintf(stderr,
                "Error: unable to allocate memory for the sieve below %zu.\n",
                limit);
        return EXIT_FAILURE;
    }

    if (!monotonic_seconds(&end_time)) {
        fputs("Error: unable to read the monotonic clock.\n", stderr);
        free(result.is_composite);
        return EXIT_FAILURE;
    }

    printf("Input limit: %zu\n", limit);
    printf("Prime count: %zu\n", result.prime_count);
    printf("Computation time: %.9f seconds\n", end_time - start_time);

    if (limit <= TERMINAL_OUTPUT_LIMIT) {
        output_ok = print_primes(limit, &result);
    } else {
        output_ok = build_output_path(argv[0], output_path,
                                      sizeof(output_path));
        if (!output_ok) {
            fputs("Error: unable to construct the prime output path.\n",
                  stderr);
        } else {
            output_ok = save_primes(output_path, &result);
        }
        if (output_ok) {
            printf("Primes written to %s\n", output_path);
        }
    }

    free(result.is_composite);
    return output_ok ? EXIT_SUCCESS : EXIT_FAILURE;
}

/* Read one complete non-negative integer and reject trailing input. */
static int read_limit(size_t *limit)
{
    char buffer[INPUT_BUFFER_SIZE];
    char *cursor;
    char *end;
    uintmax_t parsed;

    fputs("Enter upper limit n: ", stdout);
    fflush(stdout);

    if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
        fputs("Error: failed to read n.\n", stderr);
        return 0;
    }
    if (strchr(buffer, '\n') == NULL && !feof(stdin)) {
        fputs("Error: input is too long.\n", stderr);
        return 0;
    }

    cursor = buffer;
    while (isspace((unsigned char)*cursor)) {
        ++cursor;
    }

    if (*cursor == '-') {
        fputs("Error: n must be a non-negative integer.\n", stderr);
        return 0;
    }

    errno = 0;
    parsed = strtoumax(cursor, &end, 10);
    if (cursor == end || errno == ERANGE || parsed > (uintmax_t)SIZE_MAX) {
        fputs("Error: n is not a supported non-negative integer.\n", stderr);
        return 0;
    }

    while (isspace((unsigned char)*end)) {
        ++end;
    }
    if (*end != '\0') {
        fputs("Error: enter only one non-negative integer.\n", stderr);
        return 0;
    }

    *limit = (size_t)parsed;
    return 1;
}

/* Use a wall clock so timings remain comparable with parallel tasks. */
static int monotonic_seconds(double *seconds)
{
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

    *seconds = (double)timestamp.tv_sec +
               (double)timestamp.tv_nsec / 1000000000.0;
#endif
    return 1;
}

/*
 * Index i represents the odd number 2*i + 3. Starting at p*p and advancing
 * by p indices marks only odd multiples, halving the usual sieve storage.
 */
static int run_odd_sieve(size_t limit, SieveResult *result)
{
    size_t index;

    result->odd_count = (limit > 2) ? (limit - 2) / 2 : 0;
    result->prime_count = (limit > 2) ? 1 : 0;
    result->is_composite = NULL;

    if (result->odd_count == 0) {
        return 1;
    }

    result->is_composite = calloc(result->odd_count,
                                  sizeof(*result->is_composite));
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
        for (; multiple_index < result->odd_count;
             multiple_index += prime) {
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

static int print_primes(size_t limit, const SieveResult *result)
{
    size_t index;

    if (printf("Primes less than %zu:", limit) < 0) {
        return 0;
    }
    if (limit > 2 && printf(" 2") < 0) {
        return 0;
    }

    for (index = 0; index < result->odd_count; ++index) {
        if (!result->is_composite[index] &&
            printf(" %zu", odd_value(index)) < 0) {
            return 0;
        }
    }

    if (putchar('\n') == EOF) {
        return 0;
    }
    return 1;
}

/*
 * __FILE__ retains the source path passed to the compiler. If that path has
 * no directory, the executable path is the best available portable fallback;
 * the documented build places the executable beside task1.c.
 */
static int build_output_path(const char *program_path, char *output_path,
                             size_t output_path_size)
{
    const char *reference_path = __FILE__;
    const char *separator = last_path_separator(reference_path);
    size_t directory_length;
    size_t file_name_length = strlen(OUTPUT_FILE_BASENAME);

    if (separator == NULL) {
        reference_path = program_path;
        separator = last_path_separator(reference_path);
    }

    directory_length = (separator == NULL)
                           ? 0
                           : (size_t)(separator - reference_path) + 1;
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

static int save_primes(const char *output_path, const SieveResult *result)
{
    FILE *output;
    size_t index;
    int write_ok = 1;

    output = fopen(output_path, "w");
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

static const char *last_path_separator(const char *path)
{
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

static size_t odd_value(size_t index)
{
    return 2 * index + 3;
}
