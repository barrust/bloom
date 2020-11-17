#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "minunit.h"
#include "../src/bloom.h"


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
*   Test set and check
*******************************************************************************/
MU_TEST(test_bloom_set) {
    int errors = 0;
    for (int i = 0; i < 3000; ++i) {
        char key[5] = {0};
        sprintf(key, "%d", i);
        errors += bloom_filter_add_string(&b, key) == BLOOM_SUCCESS ? 0 : 1;
    }
    mu_assert_int_eq(0, errors);
    mu_assert_int_eq(3000, b.elements_added);

    errors = 0;
    for (int i = 0; i < 3000; ++i) {
        char key[5] = {0};
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
        char key[5] = {0};
        sprintf(key, "%d", i);
        errors += bloom_filter_add_string(&bf, key) == BLOOM_SUCCESS ? 0 : 1;
    }
    mu_assert_int_eq(0, errors);
    mu_assert_int_eq(3000, bf.elements_added);

    errors = 0;
    for (int i = 0; i < 3000; ++i) {
        char key[5] = {0};
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
        char key[5] = {0};
        sprintf(key, "%d", i);
        errors += bloom_filter_add_string(&b, key) == BLOOM_SUCCESS ? 0 : 1;
    }

    /* check things that are not present */
    errors = 0;
    for (int i = 3000; i < 5000; ++i) {
        char key[5] = {0};
        sprintf(key, "%d", i);
        errors += bloom_filter_check_string(&b, key) == BLOOM_FAILURE ? 0 : 1;
    }
    mu_assert_int_eq(0, errors);
}

MU_TEST(test_bloom_check_false_positive) {
    int errors = 0;
    for (int i = 0; i < 50000; ++i) {
        char key[5] = {0};
        sprintf(key, "%d", i);
        errors += bloom_filter_add_string(&b, key) == BLOOM_SUCCESS ? 0 : 1;
    }

    /* check things that are not present */
    errors = 0;
    for (int i = 50000; i < 51000; ++i) {
        char key[5] = {0};
        sprintf(key, "%d", i);
        errors += bloom_filter_check_string(&b, key) == BLOOM_FAILURE ? 0 : 1;
    }
    mu_assert_int_eq(11, errors);  // there are 11 false positives!
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
        char key[5] = {0};
        sprintf(key, "%d", i);
        bloom_filter_add_string(&b, key);
    }
    mu_assert_int_eq(5000, b.elements_added);

    mu_assert_int_eq(BLOOM_SUCCESS, bloom_filter_clear(&b));

    mu_assert_int_eq(0, b.elements_added);

    int errors = 0;
    for (int i = 0; i < 5000; ++i) {
        char key[5] = {0};
        sprintf(key, "%d", i);
        errors += bloom_filter_check_string(&b, key) == BLOOM_SUCCESS ? 0 : 1;
    }
    mu_assert_int_eq(5000, errors);
}

/*******************************************************************************
*   Test statistics
*******************************************************************************/
MU_TEST(test_bloom_current_false_positive_rate) {
    mu_assert_double_eq(0.00000, bloom_filter_current_false_positive_rate(&b));

    for (int i = 0; i < 5000; ++i) {
        char key[5] = {0};
        sprintf(key, "%d", i);
        bloom_filter_add_string(&b, key);
    }

    mu_assert_double_between(0.0000, 0.0010, bloom_filter_current_false_positive_rate(&b));

    for (int i = 5000; i < 50000; ++i) {
        char key[5] = {0};
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
        char key[5] = {0};
        sprintf(key, "%d", i);
        bloom_filter_add_string(&b, key);
    }
    mu_assert_int_eq(33592, bloom_filter_count_set_bits(&b));
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
        char key[5] = {0};
        sprintf(key, "%d", i);
        bloom_filter_add_string(&b, key);
    }
    mu_assert_int_eq(5000, b.elements_added);
    mu_assert_int_eq(4974, bloom_filter_estimate_elements(&b));

    for (int i = 5000; i < 10000; ++i) {
        char key[5] = {0};
        sprintf(key, "%d", i);
        bloom_filter_add_string(&b, key);
    }
    mu_assert_int_eq(10000, b.elements_added);
    mu_assert_int_eq(9960, bloom_filter_estimate_elements(&b));
}

MU_TEST(test_bloom_set_elements_to_estimated) {
    /*  same test as estimate elements but will then update it at the end
        to make sure we can update it. This function is useful for the set
        operations of bloom filters */
    for (int i = 0; i < 5000; ++i) {
        char key[5] = {0};
        sprintf(key, "%d", i);
        bloom_filter_add_string(&b, key);
    }
    mu_assert_int_eq(5000, b.elements_added);
    mu_assert_int_eq(4974, bloom_filter_estimate_elements(&b));

    for (int i = 5000; i < 10000; ++i) {
        char key[5] = {0};
        sprintf(key, "%d", i);
        bloom_filter_add_string(&b, key);
    }
    mu_assert_int_eq(10000, b.elements_added);
    mu_assert_int_eq(9960, bloom_filter_estimate_elements(&b));
    bloom_filter_set_elements_to_estimated(&b);
    mu_assert_int_eq(9960, b.elements_added);
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

    /* set and contains */
    MU_RUN_TEST(test_bloom_set);
    MU_RUN_TEST(test_bloom_set_failure);
    MU_RUN_TEST(test_bloom_set_on_disk);
    MU_RUN_TEST(test_bloom_check);
    MU_RUN_TEST(test_bloom_check_false_positive);
    MU_RUN_TEST(test_bloom_check_failure);

    /* clear, reset */
    MU_RUN_TEST(test_bloom_clear);

    /* statistics */
    MU_RUN_TEST(test_bloom_current_false_positive_rate);
    MU_RUN_TEST(test_bloom_count_set_bits);
    MU_RUN_TEST(test_bloom_export_size);
    MU_RUN_TEST(test_bloom_estimate_elements);
    MU_RUN_TEST(test_bloom_set_elements_to_estimated);
}


int main() {
    // we want to ignore stderr print statements
    freopen("/dev/null", "w", stderr);

    MU_RUN_SUITE(test_suite);
    MU_REPORT();
    printf("Number failed tests: %d\n", minunit_fail);
    return minunit_fail;
}
