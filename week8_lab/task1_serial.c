/*
 * FIT3143 Lab 1 / Lab 2 Serial Baseline
 *
 * Serial prime search used as the baseline for MPI speedup.
 *
 * Finds every prime strictly below n using odd-only trial division.
 * Each candidate is tested only by odd divisors up to sqrt(candidate).
 *
 * Team members:
 * - Savin Vindiv De Alwis, 35221631, sdea0018@student.monash.edu
 * - Willwara Arachchilage Shamle Imal Thilaksiri, 35512075,wthi0003@student.monash.edu
 *
 * Build:
 * gcc -std=c11 -O3 -Wall -Wextra -pedantic \
 *     task1_serial.c -o task1_serial -lm
 *
 * Run:
 * ./task1_serial 30
 *
 * Benchmark:
 * ./task1_serial --benchmark 10000000
 */

#define _POSIX_C_SOURCE 200809L

#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define TERMINAL_OUTPUT_LIMIT 100
#define OUTPUT_FILE_BASENAME "task1_serial_primes.txt"

typedef struct {
    unsigned char *is_prime;
    size_t odd_count;
    size_t prime_count;
} PrimeResult;

/* --------------------------------------------------------- */
/* Function declarations                                     */
/* --------------------------------------------------------- */

static int get_options(
    int argc,
    char *argv[],
    size_t *limit,
    int *benchmark_mode
);

static int parse_size_value(
    const char *text,
    const char *name,
    size_t minimum,
    size_t maximum,
    size_t *value
);

static int allocate_result(
    size_t limit,
    PrimeResult *result
);

static void search_serial(
    size_t limit,
    PrimeResult *result
);

static int is_prime_by_trial_division(
    size_t candidate
);

static int monotonic_seconds(
    double *seconds
);

static int print_primes(
    size_t limit,
    const PrimeResult *result
);

static int save_primes(
    const char *output_path,
    size_t limit,
    const PrimeResult *result
);

static size_t odd_value(
    size_t index
);

/* --------------------------------------------------------- */
/* Main                                                      */
/* --------------------------------------------------------- */

int main(int argc, char *argv[])
{
    PrimeResult result = {NULL, 0, 0};

    size_t limit = 0;

    int benchmark_mode = 0;
    int output_ok = 1;

    double total_start;
    double total_end;

    double computation_start;
    double computation_end;

    /* ----------------------------------------------------- */
    /* Parse command-line input                              */
    /* ----------------------------------------------------- */

    if (!get_options(
            argc,
            argv,
            &limit,
            &benchmark_mode)) {

        return EXIT_FAILURE;
    }

    /*
     * Start overall wall-clock timing here.
     *
     * This includes:
     *
     * - result allocation
     * - prime computation
     * - output file writing
     *
     * Input parsing and final summary printing are excluded.
     */
    if (!monotonic_seconds(&total_start)) {

        fputs(
            "Error: unable to read the monotonic clock.\n",
            stderr
        );

        return EXIT_FAILURE;
    }

    /* ----------------------------------------------------- */
    /* Allocate serial result storage                        */
    /* ----------------------------------------------------- */

    if (!allocate_result(limit, &result)) {

        fprintf(
            stderr,
            "Error: unable to allocate the prime flags below %zu.\n",
            limit
        );

        return EXIT_FAILURE;
    }

    /* ----------------------------------------------------- */
    /* Measure computation separately                        */
    /* ----------------------------------------------------- */

    if (!monotonic_seconds(&computation_start)) {

        fputs(
            "Error: unable to read the monotonic clock.\n",
            stderr
        );

        free(result.is_prime);

        return EXIT_FAILURE;
    }

    search_serial(
        limit,
        &result
    );

    if (!monotonic_seconds(&computation_end)) {

        fputs(
            "Error: unable to read the monotonic clock.\n",
            stderr
        );

        free(result.is_prime);

        return EXIT_FAILURE;
    }

    /* ----------------------------------------------------- */
    /* Output                                                */
    /* ----------------------------------------------------- */

    /*
     * Benchmark mode:
     *
     * Always write the prime list to a file.
     *
     * This is intentional because the MPI implementation
     * also includes file writing in its overall wall time.
     */
    if (benchmark_mode) {

        output_ok =
            save_primes(
                OUTPUT_FILE_BASENAME,
                limit,
                &result
            );
    }

    /*
     * Normal mode:
     *
     * For tiny examples, print the primes to the terminal.
     * For larger inputs, save them to a file.
     */
    else {

        if (limit < TERMINAL_OUTPUT_LIMIT) {

            output_ok =
                print_primes(
                    limit,
                    &result
                );
        }

        else {

            output_ok =
                save_primes(
                    OUTPUT_FILE_BASENAME,
                    limit,
                    &result
                );
        }
    }

    /* ----------------------------------------------------- */
    /* Finish overall wall-clock timing                      */
    /* ----------------------------------------------------- */

    if (!monotonic_seconds(&total_end)) {

        fputs(
            "Error: unable to read the monotonic clock.\n",
            stderr
        );

        free(result.is_prime);

        return EXIT_FAILURE;
    }

    /* ----------------------------------------------------- */
    /* Summary                                               */
    /* ----------------------------------------------------- */

    printf("\n");
    printf("Implementation: serial\n");
    printf("Input limit: %zu\n", limit);
    printf("Prime count: %zu\n", result.prime_count);

    printf(
        "Computation time: %.9f seconds\n",
        computation_end - computation_start
    );

    printf(
        "Overall wall-clock time: %.9f seconds\n",
        total_end - total_start
    );

    if (benchmark_mode ||
        limit >= TERMINAL_OUTPUT_LIMIT) {

        printf(
            "Output file: %s\n",
            OUTPUT_FILE_BASENAME
        );
    }

    free(result.is_prime);

    return output_ok
               ? EXIT_SUCCESS
               : EXIT_FAILURE;
}

/* --------------------------------------------------------- */
/* Command-line parsing                                      */
/* --------------------------------------------------------- */

static int get_options(
    int argc,
    char *argv[],
    size_t *limit,
    int *benchmark_mode
)
{
    /*
     * Normal mode:
     *
     * ./task1_serial n
     */
    if (argc == 2) {

        *benchmark_mode = 0;

        return parse_size_value(
            argv[1],
            "n",
            0,
            SIZE_MAX,
            limit
        );
    }

    /*
     * Benchmark mode:
     *
     * ./task1_serial --benchmark n
     */
    if (argc == 3 &&
        strcmp(argv[1], "--benchmark") == 0) {

        *benchmark_mode = 1;

        return parse_size_value(
            argv[2],
            "n",
            0,
            SIZE_MAX,
            limit
        );
    }

    fprintf(
        stderr,
        "Usage:\n"
        "  %s n\n"
        "  %s --benchmark n\n",
        argv[0],
        argv[0]
    );

    return 0;
}

static int parse_size_value(
    const char *text,
    const char *name,
    size_t minimum,
    size_t maximum,
    size_t *value
)
{
    const char *cursor = text;

    char *end;

    uintmax_t parsed;

    while (isspace((unsigned char)*cursor)) {
        ++cursor;
    }

    if (*cursor == '-') {

        fprintf(
            stderr,
            "Error: %s must not be negative.\n",
            name
        );

        return 0;
    }

    errno = 0;

    parsed =
        strtoumax(
            cursor,
            &end,
            10
        );

    if (cursor == end ||
        errno == ERANGE ||
        parsed > (uintmax_t)SIZE_MAX) {

        fprintf(
            stderr,
            "Error: %s is not a supported integer.\n",
            name
        );

        return 0;
    }

    while (isspace((unsigned char)*end)) {
        ++end;
    }

    if (*end != '\0') {

        fprintf(
            stderr,
            "Error: enter only one integer for %s.\n",
            name
        );

        return 0;
    }

    if (parsed < (uintmax_t)minimum ||
        parsed > (uintmax_t)maximum) {

        fprintf(
            stderr,
            "Error: %s must be between %zu and %zu.\n",
            name,
            minimum,
            maximum
        );

        return 0;
    }

    *value =
        (size_t)parsed;

    return 1;
}

/* --------------------------------------------------------- */
/* Result allocation                                         */
/* --------------------------------------------------------- */

static int allocate_result(
    size_t limit,
    PrimeResult *result
)
{
    /*
     * Only odd candidates >= 3 need storage.
     */
    result->odd_count =
        (limit > 2)
            ? (limit - 2) / 2
            : 0;

    result->prime_count = 0;
    result->is_prime = NULL;

    if (result->odd_count == 0) {
        return 1;
    }

    result->is_prime =
        malloc(
            result->odd_count *
            sizeof(*result->is_prime)
        );

    return result->is_prime != NULL;
}

/* --------------------------------------------------------- */
/* Serial prime search                                       */
/* --------------------------------------------------------- */

static void search_serial(
    size_t limit,
    PrimeResult *result
)
{
    size_t prime_count =
        (limit > 2)
            ? 1
            : 0;

    for (size_t index = 0;
         index < result->odd_count;
         ++index) {

        size_t candidate =
            odd_value(index);

        int is_prime =
            is_prime_by_trial_division(
                candidate
            );

        result->is_prime[index] =
            (unsigned char)is_prime;

        prime_count +=
            (size_t)is_prime;
    }

    result->prime_count =
        prime_count;
}

/* --------------------------------------------------------- */
/* Prime testing                                             */
/* --------------------------------------------------------- */

static int is_prime_by_trial_division(
    size_t candidate
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

    size_t divisor_limit =
        (size_t)sqrt(
            (double)candidate
        );

    for (size_t divisor = 3;
         divisor <= divisor_limit;
         divisor += 2) {

        if (candidate % divisor == 0) {
            return 0;
        }
    }

    return 1;
}

/* --------------------------------------------------------- */
/* Timing                                                    */
/* --------------------------------------------------------- */

static int monotonic_seconds(
    double *seconds
)
{
    struct timespec timestamp;

    if (clock_gettime(
            CLOCK_MONOTONIC,
            &timestamp) != 0) {

        return 0;
    }

    *seconds =
        (double)timestamp.tv_sec +
        (double)timestamp.tv_nsec /
            1000000000.0;

    return 1;
}

/* --------------------------------------------------------- */
/* Output                                                    */
/* --------------------------------------------------------- */

static int print_primes(
    size_t limit,
    const PrimeResult *result
)
{
    if (printf(
            "Primes less than %zu:",
            limit) < 0) {

        return 0;
    }

    if (limit > 2) {

        if (printf(" 2") < 0) {
            return 0;
        }
    }

    for (size_t index = 0;
         index < result->odd_count;
         ++index) {

        if (result->is_prime[index]) {

            if (printf(
                    " %zu",
                    odd_value(index)) < 0) {

                return 0;
            }
        }
    }

    return putchar('\n') != EOF;
}

static int save_primes(
    const char *output_path,
    size_t limit,
    const PrimeResult *result
)
{
    FILE *output =
        fopen(
            output_path,
            "w"
        );

    if (output == NULL) {

        perror(
            "Error opening prime output file"
        );

        return 0;
    }

    int write_ok = 1;

    /*
     * 2 is the only even prime.
     */
    if (limit > 2) {

        if (fprintf(
                output,
                "2\n") < 0) {

            write_ok = 0;
        }
    }

    for (size_t index = 0;
         write_ok &&
         index < result->odd_count;
         ++index) {

        if (result->is_prime[index]) {

            if (fprintf(
                    output,
                    "%zu\n",
                    odd_value(index)) < 0) {

                write_ok = 0;
            }
        }
    }

    if (fclose(output) == EOF) {
        write_ok = 0;
    }

    if (!write_ok) {

        fputs(
            "Error: failed while writing the prime output file.\n",
            stderr
        );
    }

    return write_ok;
}

/* --------------------------------------------------------- */
/* Odd-index mapping                                         */
/* --------------------------------------------------------- */

static size_t odd_value(
    size_t index
)
{
    return 2 * index + 3;
}

