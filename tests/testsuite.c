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
*   Test Import / Export
*******************************************************************************/
MU_TEST(test_bloom_export) {
    char filepath[] = "./dist/test_bloom_export.blm";
    for (int i = 0; i < 5000; ++i) {
        char key[5] = {0};
        sprintf(key, "%d", i);
        bloom_filter_add_string(&b, key);
    }

    mu_assert_int_eq(BLOOM_SUCCESS, bloom_filter_export(&b, filepath));

    char digest[33] = {0};
    calculate_md5sum(filepath, digest);
    mu_assert_string_eq("0e3b2c3c86bed868bc1da526e3747597", digest);
    mu_assert_int_eq(fsize(filepath), 59927);
    remove(filepath);
}

MU_TEST(test_bloom_export_on_disk) {
    // exporting on disk just closes the file!
    char filepath[] = "./dist/test_bloom_export.blm";

    BloomFilter bf;
    bloom_filter_init_on_disk(&bf, 50000, 0.01, filepath);

    for (int i = 0; i < 5000; ++i) {
        char key[5] = {0};
        sprintf(key, "%d", i);
        bloom_filter_add_string(&bf, key);
    }

    mu_assert_int_eq(BLOOM_SUCCESS, bloom_filter_export(&bf, filepath));

    bloom_filter_destroy(&bf);

    char digest[33] = {0};
    calculate_md5sum(filepath, digest);
    mu_assert_string_eq("0e3b2c3c86bed868bc1da526e3747597", digest);
    mu_assert_int_eq(fsize(filepath), 59927);
    remove(filepath);
}

MU_TEST(test_bloom_import) {
    char filepath[] = "./dist/test_bloom_import.blm";
    for (int i = 0; i < 5000; ++i) {
        char key[5] = {0};
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
        char key[5] = {0};
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
        char key[5] = {0};
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
        char key[5] = {0};
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
/*
MU_TEST(test_bloom_export_hex) {

}

MU_TEST(test_bloom_import_hex) {

}

MU_TEST(test_bloom_import_hex_fail) {

}
*/

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

    /* export, import */
    MU_RUN_TEST(test_bloom_export);
    MU_RUN_TEST(test_bloom_export_on_disk);
    MU_RUN_TEST(test_bloom_import);
    MU_RUN_TEST(test_bloom_import_fail);
    MU_RUN_TEST(test_bloom_import_on_disk);
    MU_RUN_TEST(test_bloom_import_on_disk_fail);
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
