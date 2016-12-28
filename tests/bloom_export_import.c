
#include <stdlib.h>
#include <stdio.h>
#include "../bloom.h"


int main(int argc, char** argv) {

	printf("Testing BloomFilter version %s\n\n", bloom_filter_get_version());
	BloomFilter bf;
	bloom_filter_init(&bf, 10, 0.05);
	bloom_filter_add_string(&bf, "test");
	bloom_filter_add_string(&bf, "test123");
	bloom_filter_add_string(&bf, "abc");
	bloom_filter_add_string(&bf, "def");
	bloom_filter_add_string(&bf, "something");
	bloom_filter_add_string(&bf, "to");
	bloom_filter_add_string(&bf, "say");
	bloom_filter_add_string(&bf, "please");
	bloom_filter_add_string(&bf, "???");
	bloom_filter_stats(&bf);

	printf("Export the bloom filter to file\n");
	bloom_filter_export(&bf, "./dist/test.blm");
	bloom_filter_destroy(&bf);

	printf("Importing the bloom filter into memory\n");
	BloomFilter bf1;
	bloom_filter_import(&bf1, "./dist/test.blm");
	bloom_filter_stats(&bf1);

	if (bloom_filter_check_string(&bf1, "test") == BLOOM_FAILURE) {
		printf("'test' is not in the bloom filter!\n");
	} else {
		printf("'test' is in the bloom filter!\n");
	}
	if (bloom_filter_check_string(&bf1, "blah") == BLOOM_FAILURE) {
		printf("'blah' is not in the bloom filter!\n");
	} else {
		printf("'blah' is in th bloom filter!\n");
	}

	bloom_filter_destroy(&bf1);

}
