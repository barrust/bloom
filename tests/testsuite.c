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
    MU_RUN_TEST(test_bloom_check);
    MU_RUN_TEST(test_bloom_check_false_positive);

}


int main() {
    // we want to ignore stderr print statements
    freopen("/dev/null", "w", stderr);

    MU_RUN_SUITE(test_suite);
    MU_REPORT();
    printf("Number failed tests: %d\n", minunit_fail);
    return minunit_fail;
}
