
#include "bloom.h"
#include <openssl/sha.h>
/*
	Example of generating a custom hashing function
*/
uint64_t* sha256_hash(int num_hashes, char *str) {
	uint64_t *results = calloc(num_hashes, sizeof(uint64_t));
	unsigned char digest[SHA256_DIGEST_LENGTH];
	int i;
	for (i = 0; i < num_hashes; i++) {
		SHA256_CTX sha256_ctx;
		SHA256_Init(&sha256_ctx);
		if (i == 0) {
			SHA256_Update(&sha256_ctx, str, strlen(str));
		} else {
			SHA256_Update(&sha256_ctx, digest, SHA256_DIGEST_LENGTH);
		}
		SHA256_Final(digest, &sha256_ctx);
		results[i] = (uint64_t) *(uint64_t *)digest;
	}
	return results;
}


int main(int argc, char** argv) {
	printf("Testing BloomFilter version %s\n\n", bloom_filter_get_version());

	BloomFilter bf;
	bloom_filter_init_alt(&bf, 10, 0.05, &sha256_hash);
	bloom_filter_add_string(&bf, "test");
	bloom_filter_add_string(&bf, "test123");
	bloom_filter_add_string(&bf, "abc");
	bloom_filter_add_string(&bf, "def");
	bloom_filter_add_string(&bf, "something");
	bloom_filter_add_string(&bf, "to");
	bloom_filter_add_string(&bf, "say");
	bloom_filter_add_string(&bf, "please");
	bloom_filter_add_string(&bf, "???");

	if (bloom_filter_check_string(&bf, "test") == BLOOM_FAILURE) {
		printf("'test' is not in the bloom filter!\n");
	} else {
		printf("'test' is in the bloom filter!\n");
	}
	if (bloom_filter_check_string(&bf, "blah") == BLOOM_FAILURE) {
		printf("'blah' is not in the bloom filter!\n");
	} else {
		printf("'blah' is in the bloom filter!\n");
	}
	bloom_filter_stats(&bf);
	char* hex = bloom_filter_export_hex_string(&bf);
	bloom_filter_destroy(&bf);

	// import the hex string and test it
	printf("\n\nImport the hex string: %s\n", hex);
	BloomFilter bf1;
	bloom_filter_import_hex_string_alt(&bf1, hex, &sha256_hash);
	free(hex);
	if (bloom_filter_check_string(&bf1, "test") == BLOOM_FAILURE) {
		printf("'test' is not in the bloom filter!\n");
	} else {
		printf("'test' is in the bloom filter!\n");
	}
	if (bloom_filter_check_string(&bf1, "blah") == BLOOM_FAILURE) {
		printf("'blah' is not in the bloom filter!\n");
	} else {
		printf("'blah' is in the bloom filter!\n");
	}
	bloom_filter_stats(&bf1);

	/* test clearing out the bloom filter */
	bloom_filter_clear(&bf1);
	bloom_filter_stats(&bf1);
	if (bloom_filter_check_string(&bf1, "test") == BLOOM_FAILURE) {
		printf("'test' is not in the bloom filter!\n");
	} else {
		printf("'test' is in the bloom filter!\n");
	}
	if (bloom_filter_check_string(&bf1, "blah") == BLOOM_FAILURE) {
		printf("'blah' is not in the bloom filter!\n");
	} else {
		printf("'blah' is in the bloom filter!\n");
	}

	bloom_filter_destroy(&bf1);
}
