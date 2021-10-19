#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <openssl/md5.h>

#include "minunit.h"
#include "../src/bloom.h"


static int calculate_md5sum(const char* filename, char* digest);
static off_t fsize(const char* filename);

static uint64_t* fake_hash(int num_hashes, const char *str);
static uint64_t hasher(const char *key);


BloomFilter b;

void test_setup(void) {
    bloom_filter_init(&b, 50000, 0.01);
}

void test_teardown(void) {
    bloom_filter_destroy(&b);
}


/*******************************************************************************
*   Test setup
*******************************************************************************/
MU_TEST(test_bloom_setup) {
    mu_assert_int_eq(50000, b.estimated_elements);
    float fpr = 0.01;
    mu_assert_double_eq(fpr, b.false_positive_probability);
    mu_assert_int_eq(7, b.number_hashes);
    mu_assert_int_eq(59907, b.bloom_length);
    mu_assert_int_eq(0, b.elements_added);

    mu_assert_null(b.filepointer);
    mu_assert_not_null(b.hash_function);
}

MU_TEST(test_bloom_setup_returns) {
    BloomFilter bf;
    mu_assert_int_eq(BLOOM_FAILURE, bloom_filter_init(&bf, 0, 0.01));
    mu_assert_int_eq(BLOOM_FAILURE, bloom_filter_init(&bf, 50000, 1.01));
    mu_assert_int_eq(BLOOM_FAILURE, bloom_filter_init(&bf, 50000, -0.01));
    mu_assert_int_eq(BLOOM_SUCCESS, bloom_filter_init(&bf, 50000, 0.01));
    bloom_filter_destroy(&bf);
}

MU_TEST(test_bloom_on_disk_setup) {
    char filepath[] = "./dist/test_bloom_on_disk_setup.blm";

    BloomFilter bf;
    int r = bloom_filter_init_on_disk(&bf, 50000, 0.01, filepath);

    mu_assert_int_eq(BLOOM_SUCCESS, r);
    mu_assert_int_eq(50000, bf.estimated_elements);
    float fpr = 0.01;
    mu_assert_double_eq(fpr, bf.false_positive_probability);
    mu_assert_int_eq(7, bf.number_hashes);
    mu_assert_int_eq(59907, bf.bloom_length);
    mu_assert_int_eq(0, bf.elements_added);

    mu_assert_not_null(bf.filepointer);
    mu_assert_not_null(bf.hash_function);

    bloom_filter_destroy(&bf);
    remove(filepath);
}

MU_TEST(test_bloom_on_disk_setup_returns) {
    char filepath[] = "./dist/test_bloom_on_disk_setup_returns.blm";
    BloomFilter bf;
    mu_assert_int_eq(BLOOM_FAILURE, bloom_filter_init_on_disk(&bf, 0, 0.01, filepath));
    mu_assert_int_eq(BLOOM_FAILURE, bloom_filter_init_on_disk(&bf, 50000, 1.01, filepath));
    mu_assert_int_eq(BLOOM_FAILURE, bloom_filter_init_on_disk(&bf, 50000, -0.01, filepath));
    mu_assert_int_eq(BLOOM_SUCCESS, bloom_filter_init_on_disk(&bf, 50000, 0.01, filepath));
    bloom_filter_destroy(&bf);
    remove(filepath);
}


/*******************************************************************************
*   Test hashing
*******************************************************************************/
MU_TEST(test_bloom_hashes_values) {
    uint64_t vals[] = {15902901984413996407ULL, 13757982394814800524ULL, 14025518860217559917ULL, 5646210032526140290ULL, 6127913770875964707ULL};
    uint64_t* hashes = bloom_filter_calculate_hashes(&b, "foo", 5);
    for (int i = 0; i < 5; ++i)
        mu_assert_int_eq(vals[i], hashes[i]);
    free(hashes);
}

MU_TEST(test_bloom_hashes_start_collisions) {
    // see https://github.com/barrust/pyprobables/issues/62#issue-913502606
    // the hashes start the same, but we want them to diverge for everything else
    uint64_t* hashes_foo = bloom_filter_calculate_hashes(&b, "gMPflVXtwGDXbIhP73TX", 5);
    uint64_t* hashes_bar = bloom_filter_calculate_hashes(&b, "LtHf1prlU1bCeYZEdqWf", 5);
    mu_assert_int_eq(hashes_foo[0], hashes_bar[0]);
    for (int i = 1; i < 5; ++i)
        mu_assert_int_not_eq(hashes_foo[i], hashes_bar[i]);
    free(hashes_foo);
    free(hashes_bar);
}


/*******************************************************************************
*   Test set and check
*******************************************************************************/
MU_TEST(test_bloom_set) {
    int errors = 0;
    for (int i = 0; i < 3000; ++i) {
        char key[10] = {0};
        sprintf(key, "%d", i);
        errors += bloom_filter_add_string(&b, key) == BLOOM_SUCCESS ? 0 : 1;
    }
    mu_assert_int_eq(0, errors);
    mu_assert_int_eq(3000, b.elements_added);

    errors = 0;
    for (int i = 0; i < 3000; ++i) {
        char key[10] = {0};
        sprintf(key, "%d", i);
        errors += bloom_filter_check_string(&b, key) == BLOOM_SUCCESS ? 0 : 1;
    }
    mu_assert_int_eq(0, errors);
}

MU_TEST(test_bloom_set_failure) {
    uint64_t* hashes = bloom_filter_calculate_hashes(&b, "three", 3); // we want too few!
    mu_assert_int_eq(BLOOM_FAILURE, bloom_filter_add_string_alt(&b, hashes, 3));
    free(hashes);
}

MU_TEST(test_bloom_set_on_disk) {
    /*  set on disk is different than in memory because some values have to be
        updated on disk; so this must be tested seperately */
    char filepath[] = "./dist/test_bloom_set_on_disk.blm";

    BloomFilter bf;
    bloom_filter_init_on_disk(&bf, 50000, 0.01, filepath);

    int errors = 0;
    for (int i = 0; i < 3000; ++i) {
        char key[10] = {0};
        sprintf(key, "%d", i);
        errors += bloom_filter_add_string(&bf, key) == BLOOM_SUCCESS ? 0 : 1;
    }
    mu_assert_int_eq(0, errors);
    mu_assert_int_eq(3000, bf.elements_added);

    errors = 0;
    for (int i = 0; i < 3000; ++i) {
        char key[10] = {0};
        sprintf(key, "%d", i);
        errors += bloom_filter_check_string(&bf, key) == BLOOM_SUCCESS ? 0 : 1;
    }
    mu_assert_int_eq(0, errors);
    bloom_filter_destroy(&bf);
    remove(filepath);
}

MU_TEST(test_bloom_check) {
    int errors = 0;
    for (int i = 0; i < 3000; ++i) {
        char key[10] = {0};
        sprintf(key, "%d", i);
        errors += bloom_filter_add_string(&b, key) == BLOOM_SUCCESS ? 0 : 1;
    }

    /* check things that are not present */
    errors = 0;
    for (int i = 3000; i < 5000; ++i) {
        char key[10] = {0};
        sprintf(key, "%d", i);
        errors += bloom_filter_check_string(&b, key) == BLOOM_FAILURE ? 0 : 1;
    }
    mu_assert_int_eq(0, errors);
}

MU_TEST(test_bloom_check_false_positive) {
    int errors = 0;
    for (int i = 0; i < 50000; ++i) {
        char key[10] = {0};
        sprintf(key, "%d", i);
        errors += bloom_filter_add_string(&b, key) == BLOOM_SUCCESS ? 0 : 1;
    }

    /* check things that are not present */
    errors = 0;
    for (int i = 50000; i < 51000; ++i) {
        char key[10] = {0};
        sprintf(key, "%d", i);
        errors += bloom_filter_check_string(&b, key) == BLOOM_FAILURE ? 0 : 1;
    }
    mu_assert_int_eq(5, errors);  // there are 5 false positives!
}

MU_TEST(test_bloom_check_failure) {
    uint64_t* hashes = bloom_filter_calculate_hashes(&b, "three", 3); // we want too few!
    mu_assert_int_eq(BLOOM_FAILURE, bloom_filter_check_string_alt(&b, hashes, 3));
    free(hashes);
}

/*******************************************************************************
*   Test clear/reset
*******************************************************************************/
MU_TEST(test_bloom_clear) {
    for (int i = 0; i < 5000; ++i) {
        char key[10] = {0};
        sprintf(key, "%d", i);
        bloom_filter_add_string(&b, key);
    }
    mu_assert_int_eq(5000, b.elements_added);

    mu_assert_int_eq(BLOOM_SUCCESS, bloom_filter_clear(&b));

    mu_assert_int_eq(0, b.elements_added);

    int errors = 0;
    for (int i = 0; i < 5000; ++i) {
        char key[10] = {0};
        sprintf(key, "%d", i);
        errors += bloom_filter_check_string(&b, key) == BLOOM_SUCCESS ? 0 : 1;
    }
    mu_assert_int_eq(5000, errors);
}


MU_TEST(test_bloom_clear_on_disk) {
    char filepath[] = "./dist/test_bloom_set_on_disk.blm";
    BloomFilter bf;
    bloom_filter_init_on_disk(&bf, 50000, 0.01, filepath);
    for (int i = 0; i < 5000; ++i) {
        char key[10] = {0};
        sprintf(key, "%d", i);
        bloom_filter_add_string(&bf, key);
    }
    mu_assert_int_eq(5000, bf.elements_added);

    mu_assert_int_eq(BLOOM_SUCCESS, bloom_filter_clear(&bf));

    mu_assert_int_eq(0, bf.elements_added);

    int errors = 0;
    for (int i = 0; i < 5000; ++i) {
        char key[10] = {0};
        sprintf(key, "%d", i);
        errors += bloom_filter_check_string(&bf, key) == BLOOM_SUCCESS ? 0 : 1;
    }
    mu_assert_int_eq(5000, errors);

    bloom_filter_destroy(&bf);

    // re-import the counting bloom filter to see if elements added was correctly set!
    bloom_filter_import(&bf, filepath);
    mu_assert_int_eq(0, bf.elements_added);
    bloom_filter_destroy(&bf);

    remove(filepath);
}

/*******************************************************************************
*   Test statistics
*******************************************************************************/
MU_TEST(test_bloom_current_false_positive_rate) {
    mu_assert_double_eq(0.00000, bloom_filter_current_false_positive_rate(&b));

    for (int i = 0; i < 5000; ++i) {
        char key[10] = {0};
        sprintf(key, "%d", i);
        bloom_filter_add_string(&b, key);
    }

    mu_assert_double_between(0.0000, 0.0010, bloom_filter_current_false_positive_rate(&b));

    for (int i = 5000; i < 50000; ++i) {
        char key[10] = {0};
        sprintf(key, "%d", i);
        bloom_filter_add_string(&b, key);
    }

    mu_assert_double_between(0.00990, 0.01010, bloom_filter_current_false_positive_rate(&b));
}

MU_TEST(test_bloom_count_set_bits) {
    mu_assert_int_eq(0, bloom_filter_count_set_bits(&b));

    bloom_filter_add_string(&b, "a");
    mu_assert_int_eq(b.number_hashes, bloom_filter_count_set_bits(&b));

    /* add a few keys */
    for (int i = 0; i < 5000; ++i) {
        char key[10] = {0};
        sprintf(key, "%d", i);
        bloom_filter_add_string(&b, key);
    }
    mu_assert_int_eq(32931, bloom_filter_count_set_bits(&b));
}

MU_TEST(test_bloom_export_size) {  // size is in bytes
    mu_assert_int_eq(59927, bloom_filter_export_size(&b));

    BloomFilter bf;
    bloom_filter_init(&bf, 100000, .5);
    mu_assert_int_eq(18054, bloom_filter_export_size(&bf));
    bloom_filter_destroy(&bf);

    bloom_filter_init(&bf, 100000, .1);
    mu_assert_int_eq(59927, bloom_filter_export_size(&bf));
    bloom_filter_destroy(&bf);

    bloom_filter_init(&bf, 100000, .05);
    mu_assert_int_eq(77961, bloom_filter_export_size(&bf));
    bloom_filter_destroy(&bf);

    bloom_filter_init(&bf, 100000, .01);
    mu_assert_int_eq(119834, bloom_filter_export_size(&bf));
    bloom_filter_destroy(&bf);

    bloom_filter_init(&bf, 100000, .001);
    mu_assert_int_eq(179740, bloom_filter_export_size(&bf));
    bloom_filter_destroy(&bf);
}

MU_TEST(test_bloom_estimate_elements) {
    for (int i = 0; i < 5000; ++i) {
        char key[10] = {0};
        sprintf(key, "%d", i);
        bloom_filter_add_string(&b, key);
    }
    mu_assert_int_eq(5000, b.elements_added);
    mu_assert_int_eq(4872, bloom_filter_estimate_elements(&b));

    for (int i = 5000; i < 10000; ++i) {
        char key[10] = {0};
        sprintf(key, "%d", i);
        bloom_filter_add_string(&b, key);
    }
    mu_assert_int_eq(10000, b.elements_added);
    mu_assert_int_eq(9792, bloom_filter_estimate_elements(&b));
}

MU_TEST(test_bloom_set_elements_to_estimated) {
    /*  same test as estimate elements but will then update it at the end
        to make sure we can update it. This function is useful for the set
        operations of bloom filters */
    for (int i = 0; i < 5000; ++i) {
        char key[10] = {0};
        sprintf(key, "%d", i);
        bloom_filter_add_string(&b, key);
    }
    mu_assert_int_eq(5000, b.elements_added);
    mu_assert_int_eq(4872, bloom_filter_estimate_elements(&b));

    for (int i = 5000; i < 10000; ++i) {
        char key[10] = {0};
        sprintf(key, "%d", i);
        bloom_filter_add_string(&b, key);
    }
    mu_assert_int_eq(10000, b.elements_added);
    mu_assert_int_eq(9792, bloom_filter_estimate_elements(&b));
    bloom_filter_set_elements_to_estimated(&b);
    mu_assert_int_eq(9792, b.elements_added);
}

MU_TEST(test_bloom_set_elements_to_estimated_on_disk) {
    /*  same test as estimate elements but will then update it at the end
        to make sure we can update it. This function is useful for the set
        operations of bloom filters */
    char filepath[] = "./dist/test_bloom_set_on_disk.blm";
    BloomFilter bf;
    bloom_filter_init_on_disk(&bf, 50000, 0.01, filepath);
    for (int i = 0; i < 5000; ++i) {
        char key[10] = {0};
        sprintf(key, "%d", i);
        bloom_filter_add_string(&bf, key);
    }
    mu_assert_int_eq(5000, bf.elements_added);
    mu_assert_int_eq(4872, bloom_filter_estimate_elements(&bf));

    for (int i = 5000; i < 10000; ++i) {
        char key[10] = {0};
        sprintf(key, "%d", i);
        bloom_filter_add_string(&bf, key);
    }
    mu_assert_int_eq(10000, bf.elements_added);
    mu_assert_int_eq(9792, bloom_filter_estimate_elements(&bf));
    bloom_filter_set_elements_to_estimated(&bf);
    mu_assert_int_eq(9792, bf.elements_added);

    bloom_filter_destroy(&bf);

    // re-import the counting bloom filter to see if elements added was correctly set!
    bloom_filter_import(&bf, filepath);
    mu_assert_int_eq(9792, bf.elements_added);
    bloom_filter_destroy(&bf);

    remove(filepath);
}

/*******************************************************************************
*   Test Import / Export
*******************************************************************************/
MU_TEST(test_bloom_export) {
    char filepath[] = "./dist/test_bloom_export.blm";
    for (int i = 0; i < 5000; ++i) {
        char key[10] = {0};
        sprintf(key, "%d", i);
        bloom_filter_add_string(&b, key);
    }

    mu_assert_int_eq(BLOOM_SUCCESS, bloom_filter_export(&b, filepath));

    char digest[33] = {0};
    calculate_md5sum(filepath, digest);
    mu_assert_string_eq("dff430adaf230fe3579d658c1fd3b457", digest);
    mu_assert_int_eq(fsize(filepath), 59927);
    remove(filepath);
}

MU_TEST(test_bloom_export_on_disk) {
    // exporting on disk just closes the file!
    char filepath[] = "./dist/test_bloom_export.blm";

    BloomFilter bf;
    bloom_filter_init_on_disk(&bf, 50000, 0.01, filepath);

    for (int i = 0; i < 5000; ++i) {
        char key[10] = {0};
        sprintf(key, "%d", i);
        bloom_filter_add_string(&bf, key);
    }

    mu_assert_int_eq(BLOOM_SUCCESS, bloom_filter_export(&bf, filepath));

    bloom_filter_destroy(&bf);

    char digest[33] = {0};
    calculate_md5sum(filepath, digest);
    mu_assert_string_eq("dff430adaf230fe3579d658c1fd3b457", digest);
    mu_assert_int_eq(fsize(filepath), 59927);
    remove(filepath);
}

MU_TEST(test_bloom_import) {
    char filepath[] = "./dist/test_bloom_import.blm";
    for (int i = 0; i < 5000; ++i) {
        char key[10] = {0};
        sprintf(key, "%d", i);
        bloom_filter_add_string(&b, key);
    }

    mu_assert_int_eq(BLOOM_SUCCESS, bloom_filter_export(&b, filepath));

    // now load the file back in!
    BloomFilter bf;
    bloom_filter_import(&bf, filepath);

    mu_assert_int_eq(50000, bf.estimated_elements);
    float fpr = 0.01;
    mu_assert_double_eq(fpr, bf.false_positive_probability);
    mu_assert_int_eq(7, bf.number_hashes);
    mu_assert_int_eq(59907, bf.bloom_length);
    mu_assert_int_eq(5000, bf.elements_added);
    int errors = 0;
    for (int i = 0; i < 5000; ++i) {
        char key[10] = {0};
        sprintf(key, "%d", i);
        errors += bloom_filter_check_string(&bf, key) == BLOOM_SUCCESS ? 0 : 1;
    }
    mu_assert_int_eq(0, errors);
    bloom_filter_destroy(&bf);
    remove(filepath);
}

/* NOTE: apparently import does not check all possible failures! */
MU_TEST(test_bloom_import_fail) {
    char filepath[] = "./dist/test_bloom_import_fail.blm";
    BloomFilter bf;
    mu_assert_int_eq(BLOOM_FAILURE, bloom_filter_import(&bf, filepath));
}

MU_TEST(test_bloom_import_on_disk) {
    char filepath[] = "./dist/test_bloom_import.blm";
    for (int i = 0; i < 5000; ++i) {
        char key[10] = {0};
        sprintf(key, "%d", i);
        bloom_filter_add_string(&b, key);
    }

    mu_assert_int_eq(BLOOM_SUCCESS, bloom_filter_export(&b, filepath));

    // now load the file back in!
    BloomFilter bf;
    bloom_filter_import_on_disk(&bf, filepath);

    mu_assert_int_eq(50000, bf.estimated_elements);
    float fpr = 0.01;
    mu_assert_double_eq(fpr, bf.false_positive_probability);
    mu_assert_int_eq(7, bf.number_hashes);
    mu_assert_int_eq(59907, bf.bloom_length);
    mu_assert_int_eq(5000, bf.elements_added);
    int errors = 0;
    for (int i = 0; i < 5000; ++i) {
        char key[10] = {0};
        sprintf(key, "%d", i);
        errors += bloom_filter_check_string(&bf, key) == BLOOM_SUCCESS ? 0 : 1;
    }
    mu_assert_int_eq(0, errors);
    bloom_filter_destroy(&bf);
    remove(filepath);
}

MU_TEST(test_bloom_import_on_disk_fail) {
    char filepath[] = "./dist/test_bloom_import_on_disk_fail.blm";
    BloomFilter bf;
    mu_assert_int_eq(BLOOM_FAILURE, bloom_filter_import_on_disk(&bf, filepath));
}

MU_TEST(test_bloom_export_hex) {
    char hex_start[] = "80202010000000008008068000001000800800000200800080220000200000000000002002000002";
    char hex_end[] = "1000000004021000000200601000000040020100000000000000c35000000000000013883c23d70a";

    for (int i = 0; i < 5000; ++i) {
        char key[10] = {0};
        sprintf(key, "%d", i);
        bloom_filter_add_string(&b, key);
    }
    char* hex = bloom_filter_export_hex_string(&b);

    char hs[81] = {0};
    strncpy(hs, hex, 80);
    char he[81] = {0};
    strncpy(he, hex + (119854 - 80), 80);

    mu_assert_string_eq(hex_start, hs);
    mu_assert_string_eq(hex_end, he);
    mu_assert_int_eq(119854, strlen(hex));

    free(hex);
}

MU_TEST(test_bloom_import_hex) {
    BloomFilter bo;
    bloom_filter_init(&bo, 500, 0.1);

    for (int i = 0; i < 250; ++i) {
        char key[10] = {0};
        sprintf(key, "%d", i);
        bloom_filter_add_string(&bo, key);
    }
    char* hex = bloom_filter_export_hex_string(&bo);

    BloomFilter bf;
    bloom_filter_import_hex_string(&bf, hex);

    mu_assert_int_eq(500, bf.estimated_elements);
    float fpr = 0.1;
    mu_assert_double_eq(fpr, bf.false_positive_probability);
    mu_assert_int_eq(3, bf.number_hashes);
    mu_assert_int_eq(300, bf.bloom_length);
    mu_assert_int_eq(250, bf.elements_added);
    int errors = 0;
    for (int i = 0; i < 250; ++i) {
        char key[10] = {0};
        sprintf(key, "%d", i);
        errors += bloom_filter_check_string(&bf, key) == BLOOM_SUCCESS ? 0 : 1;
    }
    mu_assert_int_eq(0, errors);

    bloom_filter_destroy(&bf);
    bloom_filter_destroy(&bo);
    free(hex);
}

/* NOTE: apparently import hex does not check all possible failures! */
MU_TEST(test_bloom_import_hex_fail) {
    BloomFilter bf;
    mu_assert_int_eq(BLOOM_FAILURE, bloom_filter_import_hex_string(&bf, (char*)"aaa"));  // only checks odd length
}

/*******************************************************************************
*   Union, Intersection, Jaccard Index
*******************************************************************************/
MU_TEST(test_bloom_filter_union_intersection_errors) {
    BloomFilter x, y, z;

    // number of hashes different
    bloom_filter_init(&x, 500, 0.1);
    bloom_filter_init(&y, 500, 0.01);
    bloom_filter_init(&z, 500, 0.1);
    mu_assert_int_eq(BLOOM_FAILURE, bloom_filter_union(&x, &y, &z));
    mu_assert_int_eq(BLOOM_FAILURE, bloom_filter_intersect(&x, &y, &z));
    mu_assert_int_eq(BLOOM_FAILURE, bloom_filter_jaccard_index(&x, &y));
    bloom_filter_destroy(&x);
    bloom_filter_destroy(&y);
    bloom_filter_destroy(&z);

    // different number of bits
    bloom_filter_init(&x, 500, 0.1);
    bloom_filter_init(&y, 505, 0.1);
    bloom_filter_init(&z, 500, 0.1);
    mu_assert_int_eq(BLOOM_FAILURE, bloom_filter_union(&x, &y, &z));
    mu_assert_int_eq(BLOOM_FAILURE, bloom_filter_intersect(&x, &y, &z));
    bloom_filter_destroy(&x);
    bloom_filter_destroy(&y);
    bloom_filter_destroy(&z);

    // different hash functions
    bloom_filter_init_alt(&x, 500, 0.1, &fake_hash);
    bloom_filter_init_alt(&y, 500, 0.1, &fake_hash);
    bloom_filter_init_alt(&z, 500, 0.1, NULL);  // use the default hash
    mu_assert_int_eq(BLOOM_FAILURE, bloom_filter_union(&x, &y, &z));
    mu_assert_int_eq(BLOOM_FAILURE, bloom_filter_intersect(&x, &y, &z));
    bloom_filter_destroy(&x);
    bloom_filter_destroy(&y);
    bloom_filter_destroy(&z);
}

MU_TEST(test_bloom_filter_union_intersection_cnt_errors) {
    BloomFilter x, y;
    bloom_filter_init(&x, 500, 0.1);
    bloom_filter_init(&y, 500, 0.01);

    mu_assert_int_eq(BLOOM_FAILURE, bloom_filter_count_intersection_bits_set(&x, &y));
    mu_assert_int_eq(BLOOM_FAILURE, bloom_filter_count_union_bits_set(&x, &y));

    bloom_filter_destroy(&x);
    bloom_filter_destroy(&y);
}

MU_TEST(test_bloom_filter_union) {
    BloomFilter x, y, z;
    bloom_filter_init(&x, 500, 0.01);
    bloom_filter_init(&y, 500, 0.01);
    bloom_filter_init(&z, 500, 0.01);

    for (int i = 0; i < 250; ++i) {
        char key_y[5] = {0};
        char key_z[5] = {0};
        sprintf(key_y, "%d", i);
        sprintf(key_z, "%d", i + 100);
        bloom_filter_add_string(&y, key_y);
        bloom_filter_add_string(&z, key_z);
    }

    int res = bloom_filter_union(&x, &y, &z);
    mu_assert_int_eq(BLOOM_SUCCESS, res);

    int errors = 0;
    for (int i = 0; i < 350; ++i) {
        char key[10] = {0};
        sprintf(key, "%d", i);
        errors += bloom_filter_check_string(&x, key) == BLOOM_SUCCESS ? 0 : 1;
    }
    mu_assert_int_eq(0, errors);
    mu_assert_int_eq(358, bloom_filter_estimate_elements(&x));
    mu_assert_int_between(350, 360, (int)x.elements_added);

    bloom_filter_destroy(&x);
    bloom_filter_destroy(&y);
    bloom_filter_destroy(&z);
}

MU_TEST(test_bloom_filter_intersection) {
    BloomFilter x, y, z;
    bloom_filter_init(&x, 500, 0.01);
    bloom_filter_init(&y, 500, 0.01);
    bloom_filter_init(&z, 500, 0.01);

    for (int i = 0; i < 250; ++i) {
        char key_y[5] = {0};
        char key_z[5] = {0};
        sprintf(key_y, "%d", i);
        sprintf(key_z, "%d", i + 100);
        bloom_filter_add_string(&y, key_y);
        bloom_filter_add_string(&z, key_z);
    }

    int res = bloom_filter_intersect(&x, &y, &z);
    mu_assert_int_eq(BLOOM_SUCCESS, res);

    int errors = 0;
    for (int i = 1500; i < 250; ++i) {
        char key[10] = {0};
        sprintf(key, "%d", i);
        errors += bloom_filter_check_string(&x, key) == BLOOM_SUCCESS ? 0 : 1;
    }
    mu_assert_int_eq(0, errors);
    mu_assert_int_eq(168, bloom_filter_estimate_elements(&x));
    mu_assert_int_between(160, 170, (int)x.elements_added);

    bloom_filter_destroy(&x);
    bloom_filter_destroy(&y);
    bloom_filter_destroy(&z);
}

MU_TEST(test_bloom_filter_interesection_57) {
    BloomFilter x, y, z;
    bloom_filter_init(&x, 16000000, 0.0010);
    bloom_filter_init(&y, 16000000, 0.0010);
    bloom_filter_init(&z, 16000000, 0.0010);

    for (int i = 0; i < 250; ++i) {
        char key_y[5] = {0};
        char key_z[5] = {0};
        sprintf(key_y, "%d", i);
        sprintf(key_z, "%d", i + 100);
        bloom_filter_add_string(&y, key_y);
        bloom_filter_add_string(&z, key_z);
    }

    int res = bloom_filter_intersect(&x, &y, &z);
    mu_assert_int_eq(BLOOM_SUCCESS, res);

    int errors = 0;
    for (int i = 1500; i < 250; ++i) {
        char key[10] = {0};
        sprintf(key, "%d", i);
        errors += bloom_filter_check_string(&x, key) == BLOOM_SUCCESS ? 0 : 1;
    }
    mu_assert_int_eq(0, errors);
    mu_assert_int_eq(150, bloom_filter_estimate_elements(&x));
    mu_assert_int_between(145, 165, (int)x.elements_added);

    mu_assert_int_eq(28755175, x.bloom_length);
    mu_assert_int_eq(230041400, x.number_bits);
    bloom_filter_destroy(&x);
    bloom_filter_destroy(&y);
    bloom_filter_destroy(&z);
}

MU_TEST(test_bloom_filter_jaccard) {
    BloomFilter y, z;
    bloom_filter_init(&y, 500, 0.01);
    bloom_filter_init(&z, 500, 0.01);

    // check that they are the same, yet empty...
    float res = bloom_filter_jaccard_index(&y, &z);
    mu_assert_double_eq(1.0, res);


    for (int i = 0; i < 400; ++i) {
        char key_y[5] = {0};
        sprintf(key_y, "%d", i);
        bloom_filter_add_string(&y, key_y);
    }

    res = bloom_filter_jaccard_index(&y, &z);
    mu_assert_double_eq(0.0, res);

    for (int i = 0; i < 100; ++i) {
        char key_z[5] = {0};
        sprintf(key_z, "%d", i);
        bloom_filter_add_string(&z, key_z);
    }
    res = bloom_filter_jaccard_index(&y, &z);
    mu_assert_double_between(0.24, 0.32, res);

    for (int i = 100; i < 200; ++i) {
        char key_z[5] = {0};
        sprintf(key_z, "%d", i);
        bloom_filter_add_string(&z, key_z);
    }
    res = bloom_filter_jaccard_index(&y, &z);
    mu_assert_double_between(0.49, 0.59, res);

    for (int i = 200; i < 300; ++i) {
        char key_z[5] = {0};
        sprintf(key_z, "%d", i);
        bloom_filter_add_string(&z, key_z);
    }
    res = bloom_filter_jaccard_index(&y, &z);
    mu_assert_double_between(0.70, 0.85, res);

    for (int i = 300; i < 400; ++i) {
        char key_z[5] = {0};
        sprintf(key_z, "%d", i);
        bloom_filter_add_string(&z, key_z);
    }
    res = bloom_filter_jaccard_index(&y, &z);
    mu_assert_double_eq(1.0, res);

    bloom_filter_destroy(&y);
    bloom_filter_destroy(&z);
}

/*******************************************************************************
*   Test Statistics
*******************************************************************************/
MU_TEST(test_bloom_filter_stat) {
    for (int i = 0; i < 400; ++i) {
        char key[10] = {0};
        sprintf(key, "%d", i);
        bloom_filter_add_string(&b, key);
    }

    /* save the printout to a buffer */
    int stdout_save;
    char buffer[2046] = {0};
    fflush(stdout); //clean everything first
    stdout_save = dup(STDOUT_FILENO); //save the stdout state
    freopen("output_file", "a", stdout); //redirect stdout to null pointer
    setvbuf(stdout, buffer, _IOFBF, 1024); //set buffer to stdout

    bloom_filter_stats(&b);

    /* reset stdout */
    freopen("output_file", "a", stdout); //redirect stdout to null again
    dup2(stdout_save, STDOUT_FILENO); //restore the previous state of stdout
    setvbuf(stdout, NULL, _IONBF, 0); //disable buffer to print to screen instantly

    // Not sure this is necessary, but it cleans it up
    remove("output_file");

    mu_assert_not_null(buffer);
    mu_assert_string_eq("BloomFilter\n\
    bits: 479253\n\
    estimated elements: 50000\n\
    number hashes: 7\n\
    max false positive rate: 0.010000\n\
    bloom length (8 bits): 59907\n\
    elements added: 400\n\
    estimated elements added: 397\n\
    current false positive rate: 0.000000\n\
    export size (bytes): 59927\n\
    number bits set: 2776\n\
    is on disk: no\n", buffer);
}


/*******************************************************************************
*   Testsuite
*******************************************************************************/
MU_TEST_SUITE(test_suite) {
    MU_SUITE_CONFIGURE(&test_setup, &test_teardown);

    /* setup */
    MU_RUN_TEST(test_bloom_setup);
    MU_RUN_TEST(test_bloom_on_disk_setup);
    MU_RUN_TEST(test_bloom_setup_returns);
    MU_RUN_TEST(test_bloom_on_disk_setup_returns);

    /* hashes */
    MU_RUN_TEST(test_bloom_hashes_values);
    MU_RUN_TEST(test_bloom_hashes_start_collisions);

    /* set and contains */
    MU_RUN_TEST(test_bloom_set);
    MU_RUN_TEST(test_bloom_set_failure);
    MU_RUN_TEST(test_bloom_set_on_disk);
    MU_RUN_TEST(test_bloom_check);
    MU_RUN_TEST(test_bloom_check_false_positive);
    MU_RUN_TEST(test_bloom_check_failure);

    /* clear, reset */
    MU_RUN_TEST(test_bloom_clear);
    MU_RUN_TEST(test_bloom_clear_on_disk);

    /* statistics */
    MU_RUN_TEST(test_bloom_current_false_positive_rate);
    MU_RUN_TEST(test_bloom_count_set_bits);
    MU_RUN_TEST(test_bloom_export_size);
    MU_RUN_TEST(test_bloom_estimate_elements);
    MU_RUN_TEST(test_bloom_set_elements_to_estimated);
    MU_RUN_TEST(test_bloom_set_elements_to_estimated_on_disk);

    /* export, import */
    MU_RUN_TEST(test_bloom_export);
    MU_RUN_TEST(test_bloom_export_on_disk);
    MU_RUN_TEST(test_bloom_import);
    MU_RUN_TEST(test_bloom_import_fail);
    MU_RUN_TEST(test_bloom_import_on_disk);
    MU_RUN_TEST(test_bloom_import_on_disk_fail);

    /* import and export hex strings */
    MU_RUN_TEST(test_bloom_export_hex);
    MU_RUN_TEST(test_bloom_import_hex);
    MU_RUN_TEST(test_bloom_import_hex_fail);

    /* Union, Intersection, Jaccard Index */
    MU_RUN_TEST(test_bloom_filter_union_intersection_errors);
    MU_RUN_TEST(test_bloom_filter_union_intersection_cnt_errors);
    MU_RUN_TEST(test_bloom_filter_union);
    MU_RUN_TEST(test_bloom_filter_intersection);
    MU_RUN_TEST(test_bloom_filter_interesection_57);
    MU_RUN_TEST(test_bloom_filter_jaccard);

    /* Statistics */
    MU_RUN_TEST(test_bloom_filter_stat);
}


int main() {
    // we want to ignore stderr print statements
    freopen("/dev/null", "w", stderr);

    MU_RUN_SUITE(test_suite);
    MU_REPORT();
    printf("Number failed tests: %d\n", minunit_fail);
    return minunit_fail;
}


static int calculate_md5sum(const char* filename, char* digest) {
    FILE *file_ptr;
    file_ptr = fopen(filename, "r");
    if (file_ptr == NULL) {
        perror("Error opening file");
        fflush(stdout);
        return 1;
    }

    int n;
    MD5_CTX c;
    char buf[512];
    ssize_t bytes;
    unsigned char out[MD5_DIGEST_LENGTH];

    MD5_Init(&c);
    do {
        bytes = fread(buf, 1, 512, file_ptr);
        MD5_Update(&c, buf, bytes);
    } while(bytes > 0);

    MD5_Final(out, &c);

    for (n = 0; n < MD5_DIGEST_LENGTH; n++) {
        char hex[3] = {0};
        sprintf(hex, "%02x", out[n]);
        digest[n*2] = hex[0];
        digest[n*2+1] = hex[1];
    }

    fclose(file_ptr);

    return 0;
}

static off_t fsize(const char* filename) {
    struct stat st;

    if (stat(filename, &st) == 0)
        return st.st_size;

    return -1;
}

static uint64_t* fake_hash(int num_hashes, const char *str) {
    uint64_t* hashes = (uint64_t*)calloc(num_hashes, sizeof(uint64_t));
    char key[17] = {0}; // largest value is 7FFF,FFFF,FFFF,FFFF
    hashes[0] = hasher(str);
    for (int i = 1; i < num_hashes; ++i) {
        sprintf(key, "%" PRIx64 "", hashes[i-1]);
        hashes[i] = hasher(key);
    }
    return hashes;
}

static uint64_t hasher(const char *key) {
    int i, len = strlen(key);
    uint64_t h = 14695981039346656073ULL; // FNV_OFFSET 64 bit
    for (i = 0; i < len; ++i){
            h = h ^ (unsigned char) key[i];
            h = h * 3; // FNV_PRIME 64 bit
    }
    return h;
}
