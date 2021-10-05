# bloom

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)
[![GitHub release](https://img.shields.io/github/v/release/barrust/bloom.svg)](https://github.com/barrust/bloom/releases)
[![C/C++ CI](https://github.com/barrust/bloom/actions/workflows/ci.yml/badge.svg?branch=master)](https://github.com/barrust/bloom/actions/workflows/ci.yml)
[![codecov](https://codecov.io/gh/barrust/bloom/branch/master/graph/badge.svg)](https://codecov.io/gh/barrust/bloom)

Bloom Filter implementation written in **C**

Bloom Filters are a probabilistic data structure that allows for the storage and
look up of elements. The data stored in a Bloom Filter is not retrievable. Once
data is 'inserted', data can be checked to see if it likely has been seen or if
it definitely has not. Bloom Filters guarantee a 0% False Negative rate with a
pre-selected false positive rate.

To use the library, copy the `src/bloom.h` and `src/bloom.c` files into your
project and include it where needed.

## License:
MIT 2015 - 2021


## Main Features:
* Set upper bound number of elements and desired false positive rate; the system
will determine number of hashes and number of bits required
* Custom hashing algorithms support
* Import and export either as file or as hex string
    * Keeps everything but the hashing algorithm
    * Hex can be used if needing to store as a string
    * File base can be loaded either on disk or into memory
* Ability to read Bloom Filter on disk instead of in memory if needed
* Add or check for presence in the filter by using either the string or hashes
    * Using hashes can be used to check many similar Bloom Filters while only
    needing to hash the string once
* Calculate current false positive rate
* Union and Intersection of Bloom Filters
* Calculate the Jaccard Index between two Bloom Filters
* **OpenMP** support for generation and lookup
    * Ensure the `bloom.c` file is compiled with `-fopenmp` along with the utilizing program


## Future Enhancements
* What would the difference between two Bloom Filters signify?


## Usage:
``` c
#include "bloom.h"

BloomFilter bf;
/*  elements = 10;
    false positive rate = 5% */
bloom_filter_init(&bf, 10, 0.05);
bloom_filter_add_string(&bf, "test");
if (bloom_filter_check_string(&bf, "test") == BLOOM_FAILURE) {
    printf("'test' is not in the Bloom Filter\n");
} else {
    printf("'test' is in the Bloom Filter\n");
}
if (bloom_filter_check_string(&bf, "blah") == BLOOM_FAILURE) {
    printf("'blah' is not in the Bloom Filter!\n");
} else {
    printf("'blah' is in th Bloom Filter\n");
}
bloom_filter_stats(&bf);
bloom_filter_destroy(&bf);
```

### User Defined Hash Function Example
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
/*  elements = 10;
    false positive rate = 5%
    custom hashing algorithm = sha256_hash function */
bloom_filter_init_alt(&bf, 10, 0.05, &sha256_hash);
bloom_filter_add_string(&bf, "test");
if (bloom_filter_check_string(&bf, "test") == BLOOM_FAILURE) {
    printf("'test' is not in the Bloom Filter\n");
} else {
    printf("'test' is in the Bloom Filter\n");
}
if (bloom_filter_check_string(&bf, "blah") == BLOOM_FAILURE) {
    printf("'blah' is not in the Bloom Filter!\n");
} else {
    printf("'blah' is in th Bloom Filter\n");
}
bloom_filter_stats(&bf);
bloom_filter_destroy(&bf);
```

## Required Compile Flags:
-lm


## Backward Compatible Hash Function
To use the older bloom filters (v1.8.2 or lower) that utilized the default hashing
algorithm, then change use the following code as the hash function:

``` c
/* NOTE: The caller will free the results */
static uint64_t* original_default_hash(unsigned int num_hashes, const char* str) {
    uint64_t *results = (uint64_t*)calloc(num_hashes, sizeof(uint64_t));
    char key[17] = {0}; // largest value is 7FFF,FFFF,FFFF,FFFF
    results[0] = __fnv_1a(str);
    for (unsigned int i = 1; i < num_hashes; ++i) {
        sprintf(key, "%" PRIx64 "", results[i-1]);
        results[i] = old_fnv_1a(key);
    }
    return results;
}

static uint64_t old_fnv_1a(const char* key) {
    // FNV-1a hash (http://www.isthe.com/chongo/tech/comp/fnv/)
    int i, len = strlen(key);
    uint64_t h = 14695981039346656073ULL; // FNV_OFFSET 64 bit
    for (i = 0; i < len; ++i){
            h = h ^ (unsigned char) key[i];
            h = h * 1099511628211ULL; // FNV_PRIME 64 bit
    }
    return h;
}
```

If using only older Bloom Filters, then you can update the // FNV_OFFSET 64 bit
to use `14695981039346656073ULL`
