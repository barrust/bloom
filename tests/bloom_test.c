/*
    Default tests for using the default hashing algorithm
*/

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>  /* roundf */
#include <string.h>
#include "timing.h"  /* URL: https://github.com/barrust/timing-c */
#include "../src/bloom.h"


#define ELEMENTS 50000
#define FALSE_POSITIVE_RATE 0.05
#define KEY_LEN 10

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KCYN  "\x1B[36m"

/* private functions */
int check_known_values(BloomFilter *bf, int mult);
int check_known_values_alt(BloomFilter *bf, int mult, int mult2, int* used);
int check_unknown_values(BloomFilter *bf, int mult);
int check_unknown_values_alt(BloomFilter *bf, int mult, int mult2, int offset, int* used);
int check_unknown_values_alt_2(BloomFilter *bf, int mult, int mult2, int offset, int* used);
void success_or_failure(int res);
void populate_bloom_filter(BloomFilter *bf, unsigned long long elements, int mult);
static uint64_t __fnv_1a_mod(char *key);
static uint64_t* __default_hash_mod(int num_hashes, char *str);



int main(int argc, char** argv) {
    Timing tm;
    timing_start(&tm);

    /* need something that can be replicated in the python version */
    BloomFilter b;
    bloom_filter_init(&b, 10, 0.050);
    bloom_filter_add_string(&b, "this is a test");
    bloom_filter_stats(&b);
    int tt;
    for (tt = 0; tt < b.bloom_length; tt++) {
        printf("%d\t", b.bloom[tt]);
    }
    printf("\n");
    bloom_filter_export(&b, "./dist/c_bloom.blm");
    char* thex = bloom_filter_export_hex_string(&b);
    printf("%s\n", thex);
    free(thex);
    bloom_filter_destroy(&b);

    printf("Testing BloomFilter version %s\n\n", bloom_filter_get_version());
    BloomFilter bf;
    // add a few additional spaces just in case!
    // bloom_filter_init_alt(&bf, ELEMENTS, FALSE_POSITIVE_RATE, &sha256_hash);
    bloom_filter_init(&bf, ELEMENTS, FALSE_POSITIVE_RATE);
    int cnt, used;
    populate_bloom_filter(&bf, ELEMENTS, 2);
    printf("Bloom Filter insertion: ");
    if (bf.false_positive_probability == (float)FALSE_POSITIVE_RATE && bf.elements_added == ELEMENTS && roundf(100 * bloom_filter_current_false_positive_rate(&bf)) / 100 <= bf.false_positive_probability) {
        success_or_failure(0);
    } else {
        success_or_failure(-1);
        // TODO: add why these failed!
    }
    printf(KCYN "NOTE:" KNRM "Bloom Filter Current False Positive Rate: %f\n", bloom_filter_current_false_positive_rate(&bf));

    bloom_filter_stats(&bf);

    printf("Bloom Filter: Check known values (all should be found): ");
    cnt = check_known_values(&bf, 2);
    success_or_failure(cnt);

    printf("Bloom Filter: Check known values (all should be either not found or false positive): ");
    cnt = check_unknown_values(&bf, 2);

    if ((float)cnt / ELEMENTS <= (float) FALSE_POSITIVE_RATE) {
        success_or_failure(0);
    } else {
        success_or_failure(-1);
    }
    printf(KCYN "NOTE:" KNRM " %d flagged as possible hits! Or %f%%\n", cnt, (float)cnt / ELEMENTS);


    printf("Bloom Filter export: ");
    int ex_res = bloom_filter_export(&bf, "./dist/test_bloom.blm");
    success_or_failure(ex_res);

    printf("Clear Bloom Filter: ");
    bloom_filter_clear(&bf);
    assert(bf.false_positive_probability == (float)FALSE_POSITIVE_RATE);
    assert(bf.elements_added == 0);  // should be empty!
    long u;
    cnt = 0;
    for(u = 0; u < bf.bloom_length; u++) {
        if(bf.bloom[u] != 0) {
            cnt++;
        }
    }
    success_or_failure(cnt);

    printf("Cleanup original Bloom Filter: ");
    bloom_filter_destroy(&bf);
    success_or_failure(0);  // there is no failure mode for destroy


    /* import in the exported bloom filter and re-run tests */
    printf("Import from file: ");
    BloomFilter bfi;
    bloom_filter_import(&bfi, "./dist/test_bloom.blm");
    if (bfi.false_positive_probability == (float)FALSE_POSITIVE_RATE && bfi.elements_added == ELEMENTS && roundf(100 * bloom_filter_current_false_positive_rate(&bfi)) / 100 <= bfi.false_positive_probability) {
        success_or_failure(0);
    } else {
        success_or_failure(-1);
        // TODO: add why these failed!
    }

    printf("Bloom Filter Imported: Check known values (all should be found): ");
    cnt = check_known_values(&bfi, 2);
    success_or_failure(cnt);


    printf("Bloom Filter Imported: Check known values (all should be either not found or false positive): ");
    cnt = check_unknown_values(&bfi, 2);
    if ((float)cnt / ELEMENTS <= (float) FALSE_POSITIVE_RATE) {
        success_or_failure(0);
    } else {
        success_or_failure(-1);
    }
    printf(KCYN "NOTE:" KNRM " %d flagged as possible hits! Or %f%%\n", cnt, (float)cnt / ELEMENTS);


    printf("Export Bloom Filter as hex string: ");
    char* bloom_hex = bloom_filter_export_hex_string(&bfi);
    // printf("\n%s\n\n\n", bloom_hex);
    if (bloom_hex != NULL) {
        success_or_failure(0);
    } else {
        success_or_failure(-1);
    }




    printf("Bloom Filter Hex Import: ");
    BloomFilter bfh;
    bloom_filter_import_hex_string(&bfh, bloom_hex);
    if (bfh.false_positive_probability == (float)FALSE_POSITIVE_RATE && bfh.elements_added == ELEMENTS && roundf(100 * bloom_filter_current_false_positive_rate(&bfh)) / 100 <= bfh.false_positive_probability) {
        success_or_failure(0);
    } else {
        success_or_failure(-1);
        // TODO: add why these failed!
    }

    printf(KCYN "NOTE:" KNRM " Free bloom hex string\n");
    free(bloom_hex);

    printf("Bloom Filter Hex: Check same as imported: ");
    int qres = 0;
    if (bfh.false_positive_probability != bfi.false_positive_probability || bfh.elements_added != bfi.elements_added || bloom_filter_current_false_positive_rate(&bfh) != bloom_filter_current_false_positive_rate(&bfi)) {
        qres = -1;
    }
    uint64_t t;
    for (t = 0; t < bfh.bloom_length; t++) {
        if (bfh.bloom[t] != bfi.bloom[t]) {
            qres = 1;
            break;
        }
    }
    if (qres == 0) {
        success_or_failure(0);
    } else {
        success_or_failure(-1);
    }

    printf("Cleanup imported Bloom Filter: ");
    bloom_filter_destroy(&bfi);
    success_or_failure(0);  // there is basically no failure mode

    printf("Bloom Filter Hex: Check known values (all should be found): ");
    cnt = check_known_values(&bfh, 2);
    success_or_failure(cnt);

    printf("Bloom Filter Hex: Check known values (all should be either not found or false positive): ");
    cnt = check_unknown_values(&bfh, 2);
    if ((float)cnt / ELEMENTS <= (float) FALSE_POSITIVE_RATE) {
        success_or_failure(0);
    } else {
        success_or_failure(-1);
    }
    printf(KCYN "NOTE:" KNRM " %d flagged as possible hits! Or %f%%\n", cnt, (float)cnt / ELEMENTS);


    printf("Cleanup hex Bloom Filter: ");
    bloom_filter_destroy(&bfh);
    success_or_failure(0);  // there is basically no failure mode


    printf("Bloom Filter initialize On Disk: ");
    BloomFilter bfd;
    bloom_filter_import_on_disk(&bfd, "./dist/test_bloom.blm");
    if (bfd.false_positive_probability == (float)FALSE_POSITIVE_RATE && bfd.elements_added == ELEMENTS && roundf(100 * bloom_filter_current_false_positive_rate(&bfd)) / 100 <= bfd.false_positive_probability) {
        success_or_failure(0);
    } else {
        success_or_failure(-1);
        // TODO: add why these failed!
    }

    printf("Bloom Filter On Disk: Check known values (all should be found): ");
    cnt = check_known_values(&bfd, 2);
    success_or_failure(cnt);

    printf("Bloom Filter On Disk: Check known values (all should be either not found or false positive): ");
    cnt = check_unknown_values(&bfd, 2);
    if ((float)cnt / ELEMENTS <= (float) FALSE_POSITIVE_RATE) {
        success_or_failure(0);
    } else {
        success_or_failure(-1);
    }
    printf(KCYN "NOTE:" KNRM " %d flagged as possible hits! Or %f%%\n", cnt, (float)cnt / ELEMENTS);

    printf("Cleanup On Disk Bloom Filter: ");
    bloom_filter_destroy(&bfd);
    success_or_failure(0);  // there is basically no failure mode



    BloomFilter res;
    BloomFilter bf1;
    BloomFilter bf2;
    printf("Bloom Filter Union / Intersection / Jaccard Index: setup Bloom Filters: ");
    bloom_filter_init(&res, ELEMENTS * 4, FALSE_POSITIVE_RATE);
    bloom_filter_init(&bf1, ELEMENTS * 4, FALSE_POSITIVE_RATE);
    bloom_filter_init(&bf2, ELEMENTS * 4, FALSE_POSITIVE_RATE);

    populate_bloom_filter(&bf1, ELEMENTS * 2, 2);
    populate_bloom_filter(&bf2, ELEMENTS * 2, 3);
    cnt = check_known_values(&bf1, 2);
    cnt += check_known_values(&bf2, 3);
    success_or_failure(cnt);

    printf("Bloom Filter Union: \n");
    printf("Bloom Filter Union: known values: ");
    bloom_filter_union(&res, &bf1, &bf2);
    cnt = check_known_values(&res, 2);
    cnt += check_known_values(&res, 3);
    success_or_failure(cnt);

    printf("Bloom Filter Union: unknown values: ");
    cnt = check_unknown_values_alt(&res, 2, 3, 11, &used);
    if ((float)cnt / used <= (float) FALSE_POSITIVE_RATE) {
        success_or_failure(0);
    } else {
        success_or_failure(-1);
    }
    printf(KCYN "NOTE:" KNRM " %d flagged as possible hits out of %d elements! Or %f%%\n", cnt, used, (float)cnt / used);
    bloom_filter_stats(&res);

    printf("Bloom Filter Union: count set bits without storing: ");
    if (bloom_filter_count_union_bits_set(&bf1, &bf2) == bloom_filter_count_set_bits(&res)) {
        success_or_failure(0);
    } else {
        success_or_failure(-1);
    }

    bloom_filter_clear(&res);
    printf("Bloom Filter Intersection: \n");
    printf("Bloom Filter Intersection: known values: ");
    bloom_filter_clear(&res);
    bloom_filter_intersect(&res, &bf1, &bf2);
    cnt = check_known_values_alt(&res, 2, 3, &used);
    success_or_failure(cnt);

    printf("Bloom Filter Intersection: unknown values: ");
    cnt = check_unknown_values_alt_2(&res, 2, 3, 23, &used);
    if ((float)cnt / used <= (float) FALSE_POSITIVE_RATE) {
        success_or_failure(0);
    } else {
        success_or_failure(-1);
    }
    printf(KCYN "NOTE:" KNRM " %d flagged as possible hits out of %d elements! Or %f%%\n", cnt, used, (float)cnt / used);
    bloom_filter_stats(&res);

    printf("Bloom Filter Intersection: reset inserted elements: ");
    if (res.elements_added == bloom_filter_estimate_elements(&res)) {
        success_or_failure(0);
    } else {
        success_or_failure(-1);
    }

    printf("Bloom Filter Intersection: count set bits without storing: ");
    if (bloom_filter_count_intersection_bits_set(&bf1, &bf2) == bloom_filter_count_set_bits(&res)) {
        success_or_failure(0);
    } else {
        success_or_failure(-1);
    }

    printf("Bloom Filter Jaccard Index: \n");
    printf("Bloom Filter Jaccard Index: same Bloom Filter: ");
    if (bloom_filter_jaccard_index(&res, &res) == 1) {
        success_or_failure(0);
    } else {
        success_or_failure(-1);
    }
    printf(KCYN "NOTE:" KNRM " similarity score: %f\n", bloom_filter_jaccard_index(&res, &res));

    printf("Bloom Filter Jaccard Index: ~30 percent similar Bloom Filter: ");
    if (bloom_filter_jaccard_index(&bf1, &bf2) < .35) {
        success_or_failure(0);
    } else {
        success_or_failure(-1);
    }
    printf(KCYN "NOTE:" KNRM " similarity score: %f\n", bloom_filter_jaccard_index(&bf1, &bf2));

    printf("Bloom Filter Jaccard Index: empty union: ");
    BloomFilter empty;
    bloom_filter_init(&empty, 500, 0.05);
    if (bloom_filter_jaccard_index(&empty, &empty) == 1.0) {
        success_or_failure(0);
    } else {
        success_or_failure(-1);
    }
    bloom_filter_destroy(&empty);


    printf("Bloom Filter Unable to Union or Intersect: \n");
    bloom_filter_init(&bf, ELEMENTS, FALSE_POSITIVE_RATE);
    printf("Bloom Filter Unable to Union or Intersect: Different number bits: ");
    if (bloom_filter_union(&bf, &bf, &bf1) == BLOOM_FAILURE && bf.number_hashes == bf1.number_hashes) {
        success_or_failure(0);
    } else {
        success_or_failure(-1);
    }

    bloom_filter_destroy(&bf2);
    bloom_filter_init(&bf2, ELEMENTS, FALSE_POSITIVE_RATE - 0.01);
    printf("Bloom Filter Unable to Union or Intersect: Different number hashes: ");
    if (bloom_filter_union(&bf, &bf, &bf2) == BLOOM_FAILURE && bf.number_hashes != bf2.number_hashes) {
        success_or_failure(0);
    } else {
        success_or_failure(-1);
    }

    // add one that uses a different hash function
    bloom_filter_destroy(&bf2);
    bloom_filter_init_alt(&bf2, ELEMENTS, FALSE_POSITIVE_RATE, &__default_hash_mod);
    printf("Bloom Filter Unable to Union or Intersect: Different hash functions: ");
    if (bloom_filter_union(&bf, &bf, &bf2) == BLOOM_FAILURE) {
        success_or_failure(0);
    } else {
        success_or_failure(-1);
    }
    printf(KCYN "NOTE:" KNRM " this is actually the same hash function, just a different location/pointer; perhaps this should really test the hash function?\n");

    printf("Cleanup Bloom Filter: ");
    bloom_filter_destroy(&bf);
    bloom_filter_destroy(&res);
    bloom_filter_destroy(&bf1);
    bloom_filter_destroy(&bf2);
    success_or_failure(0);

    timing_end(&tm);
    printf("\nCompleted Bloom Filter tests in %f seconds!\n", timing_get_difference(tm));
    printf("\nCompleted tests!\n");
}


/* private function definitions */
void populate_bloom_filter(BloomFilter *bf, unsigned long long elements, int mult) {
    int i;
    for (i = 0; i < elements * mult; i+=mult) {
        char key[KEY_LEN] = {0};
        sprintf(key, "%d", i);
        bloom_filter_add_string(bf, key);
    }
}

int check_known_values(BloomFilter *bf, int mult) {
    int i, cnt = 0;
    for (i = 0; i < ELEMENTS * mult; i+=mult) {
        char key[KEY_LEN] = {0};
        sprintf(key, "%d", i);
        if (bloom_filter_check_string(bf, key) == BLOOM_FAILURE) {
            cnt++;
        }
    }
    return cnt;
}

int check_known_values_alt(BloomFilter *bf, int mult, int mult2, int* used) {
    int i, cnt = 0;
    int j = 0;
    for (i = 0; i < ELEMENTS * mult; i+=mult) {
        if (i % mult2 == 0 && i % mult == 0) {
            char key[KEY_LEN] = {0};
            sprintf(key, "%d", i);
            if (bloom_filter_check_string(bf, key) == BLOOM_FAILURE) {
                cnt++;
            }
            j++;
        }
    }
    *used = j;
    return cnt;
}

int check_unknown_values(BloomFilter *bf, int mult) {
    int i, cnt = 0;
    for (i = 1; i < ELEMENTS * mult; i+=mult) {
        char key[KEY_LEN] = {0};
        sprintf(key, "%d", i);
        if (bloom_filter_check_string(bf, key) == BLOOM_SUCCESS) {
            cnt++;
        }
    }
    return cnt;
}

int check_unknown_values_alt(BloomFilter *bf, int mult, int mult2, int offset, int* used) {
    int i, cnt = 0;
    int j = 0;
    for (i = offset; i < ELEMENTS * offset; i+=offset) {
        if (i % mult2 == 0 || i % mult == 0) {
            // pass
        } else {
            char key[KEY_LEN] = {0};
            sprintf(key, "%d", i);
            if (bloom_filter_check_string(bf, key) == BLOOM_SUCCESS) {
                cnt++;
            }
            j++;
        }
    }
    *used = j;
    return cnt;
}

int check_unknown_values_alt_2(BloomFilter *bf, int mult, int mult2, int offset, int* used) {
    int i, cnt = 0;
    int j = 0;
    for (i = offset; i < ELEMENTS * offset; i+=offset) {
        if (i % mult2 == 0 && i % mult == 0) {
            // pass
        } else {
            char key[KEY_LEN] = {0};
            sprintf(key, "%d", i);
            if (bloom_filter_check_string(bf, key) == BLOOM_SUCCESS) {
                cnt++;
            }
            j++;
        }
    }
    *used = j;
    return cnt;
}

void success_or_failure(int res) {
    if (res == 0) {
        printf(KGRN "success!\n" KNRM);
    } else {
        printf(KRED "failure!\n" KNRM);
    }
}

/* NOTE: The caller will free the results */
static uint64_t* __default_hash_mod(int num_hashes, char *str) {
    uint64_t *results = calloc(num_hashes, sizeof(uint64_t));
    int i;
    char *key = calloc(17, sizeof(char));  // largest value is 7FFF,FFFF,FFFF,FFFF
    for (i = 0; i < num_hashes; i++) {
        if (i == 0) {
            results[i] = __fnv_1a_mod(str);
        } else {
            uint64_t prev = results[i-1];
            memset(key, 0, 17);
            sprintf(key, "%" PRIx64 "", prev);
            results[i] = __fnv_1a_mod(key);
        }
    }
    free(key);
    return results;
}

static uint64_t __fnv_1a_mod(char *key) {
    // FNV-1a hash (http://www.isthe.com/chongo/tech/comp/fnv/)
    int i, len = strlen(key);
    uint64_t h = 14695981039346656073ULL; // FNV_OFFSET 64 bit
    for (i = 0; i < len; i++){
            h = h ^ (unsigned char) key[i];
            h = h * 1099511628211ULL; // FNV_PRIME 64 bit
    }
    return h;
}
