

#include "bloom.h"

int main(int argc, char** argv) {
	printf("Testing BloomFilter version %s\n\n", bloom_filter_get_version());
	BloomFilter bf;
	bloom_filter_init_on_disk(&bf, 10000000, 0.001, "./dist/test.blm");

	printf("Completed initializing the bloom filter on disk!\n");

	int i;
	for(i = 1; i < 10000000; i++) {
		bloom_filter_add_string(&bf, (char *)&i);
	}
	bloom_filter_destroy(&bf);
	printf("Destroyed the original on disk bloom filter\n\n");

	printf("Import the exported bloom filter on disk for read / write\n");
	BloomFilter bf1;
	bloom_filter_import_on_disk(&bf1, "./dist/test.blm");

	// '100000000' should not be in the bloom filter unless it is a false positive!
	if (bloom_filter_check_string(&bf1, "100000000") == BLOOM_FAILURE) {
		printf("'100000000' is not in the bloom filter!\n");
	} else {
		printf("'100000000' is in the bloom filter!\n");
	}
	// '99' should be in the bloom filter!
	if (bloom_filter_check_string(&bf1, "99") == BLOOM_FAILURE) {
		printf("'99' is not in the bloom filter!\n");
	} else {
		printf("'99' is in the bloom filter!\n");
	}
	bloom_filter_stats(&bf1);
	bloom_filter_destroy(&bf1);
}
