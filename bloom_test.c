
#include "bloom.h"
#include <openssl/sha.h>
/*
	Example of generating a custom hashing function
*/
uint64_t* sha256_hash2(int num_hashes, uint64_t num_bits, char *str) {
	uint64_t *results = calloc(num_hashes, sizeof(uint64_t));
	unsigned char digest[64];
	int i;
	for (i = 0; i < num_hashes; i++) {
		SHA256_CTX sha256_ctx;
		SHA256_Init(&(sha256_ctx));
		if (i == 0) {
			SHA256_Update(&(sha256_ctx), str, strlen(str));
		} else {
			SHA256_Update(&(sha256_ctx), digest, 64);
		}
		SHA256_Final(digest, &(sha256_ctx));
		results[i] = (uint64_t) *(uint64_t *)digest % num_bits;
	}
	return results;
}


int main(int argc, char** argv) {
    printf("Testing BloomFilter version %s\n\n", bloom_filter_get_version());
    BloomFilter bf;
    bloom_filter_init(&bf, 10, 0.05, &sha256_hash2);
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
        printf("test is not in the bloom filter\n");
    } else {
        printf("test is in the bloom filter\n");
    }
    if (bloom_filter_check_string(&bf, "blah") == BLOOM_FAILURE) {
        printf("blah is not in the bloom filter!\n");
    }else {
        printf("blah is in th bloom filter\n");
    }
    bloom_filter_stats(&bf);
    bloom_filter_destroy(&bf);
}
