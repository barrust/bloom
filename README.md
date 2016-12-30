# bloom
Bloom Filter implementation written in C.

Bloom filters are a probabilistic data structure that allows for the storage and look up of elements. The data stored in a bloom filter is not retrievable. Once data is 'inserted', data can be checked to see if it likely has been seen or if it definitely has not. Bloom filters guarantee a 0% False Negative rate with a pre-selected false positive rate.


## License:
MIT 2015


## Main Features:
* Set upper bound number of elements and desired false positive rate; the system will determine number of hashes and number of bits required
* Custom hashing algorithms support
* Import and export either as file or as hex string
	* Keeps everything but the hashing algorithm
	* Hex can be used if needing to store as a string
	* File base can be loaded either on disk or into memory
* Ability to read bloom filter on disk instead of in memory if needed
* Add or check for presence in the filter by using either the string or hashes
    * Using hashes can be used to check many similar bloom filters while only needing to hash the string once
* Calculate current false positive rate
* **OpenMP** support for generation and lookup


## Future Enhancements
* Union and Intersection of bloom filters


## Usage:
``` c
#include "bloom.h"

BloomFilter bf;
/*
	elements = 10;
	false positive rate = 5%
*/
bloom_filter_init(&bf, 10, 0.05);
bloom_filter_add_string(&bf, "test");
if (bloom_filter_check_string(&bf, "test") == BLOOM_FAILURE) {
	printf("'test' is not in the bloom filter\n");
} else {
	printf("'test' is in the bloom filter\n");
}
if (bloom_filter_check_string(&bf, "blah") == BLOOM_FAILURE) {
	printf("'blah' is not in the bloom filter!\n");
}else {
	printf("'blah' is in th bloom filter\n");
}
bloom_filter_stats(&bf);
bloom_filter_destroy(&bf);
```

### Example User Defined Hash Function
``` c
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <openssl/sha.h>
#include "bloom.h"

/* Example of a custom hashing function */
uint64_t* sha256_hash(int num_hashes, char* str) {
	uint64_t* results = calloc(num_hashes, sizeof(uint64_t));
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
		results[i] = (uint64_t)* (uint64_t* )digest;
	}
	return results;
}

BloomFilter bf;
/*
	elements = 10;
	false positive rate = 5%
	custom hashing algorithm = sha256_hash function
*/
bloom_filter_init_alt(&bf, 10, 0.05, &sha256_hash);
bloom_filter_add_string(&bf, "test");
if (bloom_filter_check_string(&bf, "test") == BLOOM_FAILURE) {
	printf("'test' is not in the bloom filter\n");
} else {
	printf("'test' is in the bloom filter\n");
}
if (bloom_filter_check_string(&bf, "blah") == BLOOM_FAILURE) {
	printf("'blah' is not in the bloom filter!\n");
}else {
	printf("'blah' is in th bloom filter\n");
}
bloom_filter_stats(&bf);
bloom_filter_destroy(&bf);
```

## Required Compile Flags:
-lm
