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
}

/*******************************************************************************
*   Testsuite
*******************************************************************************/
MU_TEST_SUITE(test_suite) {
    MU_SUITE_CONFIGURE(&test_setup, &test_teardown);

    /* setup */
    MU_RUN_TEST(test_bloom_setup);


}


int main() {
    // we want to ignore stderr print statements
    freopen("/dev/null", "w", stderr);

    MU_RUN_SUITE(test_suite);
    MU_REPORT();
    printf("Number failed tests: %d\n", minunit_fail);
    return minunit_fail;
}
