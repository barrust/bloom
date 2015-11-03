# bloom
Bloom filter implementation written in C.

Bloom filters are a probabilistic data structure that allows for the storage and look up of elements that have previously been seen.

##License:
MIT 2015

##Usage:
```
#include "bloom.h"

BloomFilter bf;
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

##Required Compile Flags:
-lm -lcrypto
