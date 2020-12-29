
#include <stdlib.h>
#include <stdio.h>
#include "../src/bloom.h"
#include "timing.h"  /* URL: https://github.com/barrust/timing-c */
#include <omp.h>

#define SIZE 100000000

int main() {
    printf("Testing BloomFilter version %s\n\n", bloom_filter_get_version());

    int THREADS = 1;
    #if defined (_OPENMP)
    printf("OpenMP Enabled\n\n");
    THREADS =  omp_get_max_threads();
    #endif

    Timing t;
    // build one in serial in memory
    timing_start(&t);
    BloomFilter bf;
    bloom_filter_init(&bf, SIZE, 0.01);
    uint64_t i;
    for(i = 1; i < SIZE; i++) {
        char str[255] = {0};
        sprintf(str, "%" PRIu64 "", i);
        bloom_filter_add_string(&bf, str);
    }
    timing_end(&t);
    printf("Completed the single threaded Bloom Filter in %f seconds!\n", timing_get_difference(t));

    // build another one to test multi-thread
    timing_start(&t);
    BloomFilter bf2;
    //bloom_filter_init(&bf2, SIZE, 0.01, NULL);
    bloom_filter_init(&bf2, SIZE, 0.01);

    #if defined (_OPENMP)
    printf("setting the number of threads to use to: %d\n", THREADS);
    omp_set_num_threads(THREADS);
    #endif

    #pragma omp parallel for
    for(uint64_t i = 1; i < SIZE; i++) {
        char str[255] = {0};
        sprintf(str, "%" PRIu64 "", i);
        if (bloom_filter_add_string(&bf2, str) == BLOOM_FAILURE) {
            #pragma omp critical
            {
                printf("Unable to add '%s'\n", str);
            }
        }
    }
    timing_end(&t);
    #if defined (_OPENMP)
    printf("Completed the multi-threaded Bloom Filter in %f seconds using %d threads!\n", timing_get_difference(t), THREADS);
    #else
    printf("Completed the multi-threaded Bloom Filter in %f seconds!\n", timing_get_difference(t));
    #endif
    bloom_filter_stats(&bf);
    bloom_filter_stats(&bf2);

    // test to see if the bloom filters are the same
    int errors = 0;
    uint64_t chars = bf.bloom_length;
    uint64_t j;
    for (j = 0; j < chars; j++) {
        if ((int)bf.bloom[j] != (int)bf2.bloom[j]) {
            printf("total elements: %" PRIu64 "\telm: %" PRIu64 "\tbf: %d\tbf2: %d\n", chars, j, (int)bf.bloom[j], (int)bf2.bloom[j]);
            errors++;
        }
    }
    if (errors == 0) {
        printf("Completed checking the Bloom Filters! They are identical!\n");
    } else {
        printf("Completed checking the Bloom Filters! There were %d errors detected.\n", errors);
    }

    errors = 0;
    #pragma omp parallel for
    for(uint64_t i = 1; i < SIZE; i++) {
        char str[255] = {0};
        sprintf(str, "%" PRIu64 "", i);
        int r = bloom_filter_check_string(&bf2, str);
        if (r == BLOOM_FAILURE) {
            #pragma omp critical
            {
                errors++;
                printf("%" PRIu64 " was not found...\n", i);
            }
        }
    }
    printf("There were %d errors in checking the Bloom Filters\n", errors);

    // '99' should be in the Bloom Filter!
    if (bloom_filter_check_string(&bf2, "99") == BLOOM_FAILURE) {
        printf("'99' is not in the Bloom Filter!\n");
    } else {
        printf("'99' is in the Bloom Filter!\n");
    }

    if (bloom_filter_check_string(&bf2, "test") == BLOOM_FAILURE) {
        printf("'test' is not in the Bloom Filter!\n");
    } else {
        printf("'test' is in the Bloom Filter!\n");
    }

    // clean up memory
    bloom_filter_destroy(&bf);
    bloom_filter_destroy(&bf2);

    return errors;
}
