/*
    Default tests for using the default hashing algorithm
*/

#include <stdio.h>
#include <assert.h>
#include <math.h>  /* roundf */
#include "../src/bloom.h"


#define ELEMENTS 50000
#define FALSE_POSITIVE_RATE 0.05
#define KEY_LEN 10


int check_known_values(BloomFilter *bf) {
	int i, cnt = 0;
    for (i = 0; i < ELEMENTS * 2; i+=2) {
        char key[KEY_LEN] = {0};
        sprintf(key, "%d", i);
        if (bloom_filter_check_string(bf, key) == BLOOM_FAILURE) {
            cnt++;
        }
    }
	return cnt;
}

int check_unknown_values(BloomFilter *bf) {
	int i, cnt = 0;
    for (i = 1; i < ELEMENTS * 2; i+=2) {
        char key[KEY_LEN] = {0};
        sprintf(key, "%d", i);
        if (bloom_filter_check_string(bf, key) == BLOOM_SUCCESS) {
            cnt++;
        }
    }
	return cnt;
}


int main(int argc, char** argv) {
    printf("Testing BloomFilter version %s\n\n", bloom_filter_get_version());
    BloomFilter bf;
    // add a few additional spaces just in case!
    // bloom_filter_init_alt(&bf, ELEMENTS, FALSE_POSITIVE_RATE, &sha256_hash);
    bloom_filter_init(&bf, ELEMENTS, FALSE_POSITIVE_RATE);
    int i, cnt;
    for (i = 0; i < ELEMENTS * 2; i+=2) {
        char key[KEY_LEN] = {0};
        sprintf(key, "%d", i);
        bloom_filter_add_string(&bf, key);
    }
    printf("%f\n", bf.false_positive_probability);
    assert(bf.false_positive_probability == (float)FALSE_POSITIVE_RATE);
    assert(bf.elements_added == ELEMENTS);
    printf("%f\n", bloom_filter_current_false_positive_rate(&bf));
    assert(roundf(100 * bloom_filter_current_false_positive_rate(&bf)) / 100 <= bf.false_positive_probability);
    printf("Bloom Filter insertion: success!\n");


    printf("Bloom Filter: Check known values (all should be found): ");
    cnt = check_known_values(&bf);
    assert(cnt == 0);
    printf("success!\n");


    printf("Bloom Filter: Check known values (all should be either not found or false positive): ");
    cnt = check_unknown_values(&bf);
    assert((float)cnt / ELEMENTS <= (float) FALSE_POSITIVE_RATE);
    printf("success!\n");
    printf("NOTE: %d flagged as possible hits! Or %f%%\n", cnt, (float)cnt / ELEMENTS);


    printf("Bloom filter export: ");
    bloom_filter_export(&bf, "./dist/test_bloom.blm");
    printf("sucess!\n");

	printf("Clear bloom filter: ");
	bloom_filter_clear(&bf);
	assert(bf.false_positive_probability == (float)FALSE_POSITIVE_RATE);
    assert(bf.elements_added == 0);  // should be empty!
	long u;
	for(u = 0; u < bf.bloom_length; u++) {
		assert(bf.bloom[u] == 0);
	}
	printf("success!\n");

    printf("Cleanup original Bloom Filter: ");
    bloom_filter_destroy(&bf);
    printf("success!\n\n");

    /* import in the exported bloom filter and re-run tests */
    printf("Import from file: ");
    BloomFilter bfi;
    bloom_filter_import(&bfi, "./dist/test_bloom.blm");
    assert(bfi.false_positive_probability == (float)FALSE_POSITIVE_RATE);
    assert(bfi.elements_added == ELEMENTS);
    assert(roundf(100 * bloom_filter_current_false_positive_rate(&bfi)) / 100 <= bfi.false_positive_probability);
    printf("success!\n");


    printf("Bloom Filter Imported: Check known values (all should be found): ");
    cnt = check_known_values(&bfi);
    assert(cnt == 0);
    printf("success!\n");


    printf("Bloom Filter Imported: Check known values (all should be either not found or false positive): ");
    cnt = check_unknown_values(&bfi);
    assert((float)cnt / ELEMENTS <= (float) FALSE_POSITIVE_RATE);
    printf("success!\n");
    printf("NOTE: %d flagged as possible hits! Or %f%%\n", cnt, (float)cnt / ELEMENTS);


    printf("Export bloom filter as hex string: ");
    char* bloom_hex = bloom_filter_export_hex_string(&bfi);
    printf("success!\n");

    printf("Cleanup imported Bloom Filter: ");
    bloom_filter_destroy(&bfi);
    printf("success!\n\n");


    printf("Bloom Filter Hex Import: ");
    BloomFilter bfh;
    bloom_filter_import_hex_string(&bfh, bloom_hex);
    assert(bfh.false_positive_probability == (float)FALSE_POSITIVE_RATE);
    assert(bfh.elements_added == ELEMENTS);
    assert(roundf(100 * bloom_filter_current_false_positive_rate(&bfh)) / 100 <= bfh.false_positive_probability);
    printf("success!\n");


    printf("Bloom Filter Hex: Check known values (all should be found): ");
    cnt = check_known_values(&bfh);
    assert(cnt == 0);
    printf("success!\n");

    printf("Bloom Filter Hex: Check known values (all should be either not found or false positive): ");
    cnt = check_unknown_values(&bfh);
    assert((float)cnt / ELEMENTS <= (float) FALSE_POSITIVE_RATE);
    printf("success!\n");
    printf("NOTE: %d flagged as possible hits! Or %f%%\n", cnt, (float)cnt / ELEMENTS);


    printf("Cleanup hex Bloom Filter: ");
    bloom_filter_destroy(&bfh);
    printf("success!\n\n");


    printf("Bloom Filter initialize On Disk: ");
    BloomFilter bfd;
    bloom_filter_import_on_disk(&bfd, "./dist/test_bloom.blm");
    assert(bfd.false_positive_probability == (float)FALSE_POSITIVE_RATE);
    assert(bfd.elements_added == ELEMENTS);
    assert(roundf(100 * bloom_filter_current_false_positive_rate(&bfd)) / 100 <= bfd.false_positive_probability);
    printf("success!\n");

    printf("Bloom Filter On Disk: Check known values (all should be found): ");
    cnt = check_known_values(&bfd);
    assert(cnt == 0);
    printf("success!\n");

    printf("Bloom Filter On Disk: Check known values (all should be either not found or false positive): ");
    cnt = check_unknown_values(&bfd);
    assert((float)cnt / ELEMENTS <= (float) FALSE_POSITIVE_RATE);
    printf("success!\n");
    printf("NOTE: %d flagged as possible hits! Or %f%%\n", cnt, (float)cnt / ELEMENTS);

    printf("Cleanup On Disk Bloom Filter: ");
    bloom_filter_destroy(&bfd);
    printf("success!\n\n");
}
