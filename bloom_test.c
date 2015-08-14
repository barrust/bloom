
#include "bloom.h"

/*
	Example of generating a custom hashing function 
*/
uint64_t* md5_hash2(int num_hashes, uint64_t num_bits, char *str) {
	printf("in md5hash! from the caller!\n");
	uint64_t *results = calloc(num_hashes, sizeof(uint64_t));
	unsigned char digest[MD5_DIGEST_LENGTH];
	int i;
	for (i = 0; i < num_hashes; i++) {
		MD5_CTX md5_ctx;
		MD5_Init(&(md5_ctx));
		if (i == 0) {
			MD5_Update(&(md5_ctx), str, strlen(str));
		} else {
			MD5_Update(&(md5_ctx), digest, MD5_DIGEST_LENGTH);
		}
		MD5_Final(digest, &(md5_ctx));
		results[i] = (uint64_t) *(uint64_t *)digest % num_bits;
	}
	return results;
}


int main(int argc, char** argv) {
    printf("Testing BloomFilter version %s\n\n", bloom_filter_get_version());
    BloomFilter bf;
    bloom_filter_init(&bf, 10, 0.05, &md5_hash2);
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
