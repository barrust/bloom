/*******************************************************************************
***
***     Author: Tyler Barrus
***     email:  barrust@gmail.com
***
***     Version: 0.8.0
***     Purpose: Simple, yet effective, bloom filter implementation
***
***     License: MIT 2015
***
***     URL:
***
***     Usage:
***			BloomFilter bf;
***			bloom_filter_init(&bf, 100000, 0.05); // 5% false positive rate
***			bloom_filter_add_string(&bf, "google");
***			bloom_filter_add_string(&bf, "twitter");
***			if (bloom_filter_check_string(&bf, "facebook") == -1) {
***				printf("facebook is not present!\n");
***			}
***			bloom_filter_destroy(&bf);
***
***		Required Compile Flags: -lm -lcrypto
***
*******************************************************************************/
#ifndef __BLOOM_FILTER_H__
#define __BLOOM_FILTER_H__

#include <stdlib.h>
#include <inttypes.h>
#include <math.h>   		/* pow, exp */
#include <stdio.h> 			/* printf */
#include <string.h> 		/* strlen */
#include <openssl/md5.h>

#ifdef __APPLE__
	#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#define BLOOMFILTER_VERSION "0.8.0"
#define BLOOMFILTER_MAJOR 0
#define BLOOMFILTER_MINOR 8
#define BLOOMFILTER_REVISION 0

#define BLOOM_SUCCESS 0
#define BLOOM_FAILURE -1

#define bloom_filter_get_version()    (BLOOMFILTER_VERSION)

typedef uint64_t* (*HashFunction) (int num_hashes, uint64_t num_bits, char *str);

typedef struct bloom_filter {
    /* bloom parameters */
    uint64_t estimated_elements;
    float false_positive_probability;
    unsigned int number_hashes;
    uint64_t number_bits;
    /* bloom filter */
    unsigned char *bloom;
    long bloom_length;
    uint64_t elements_added;
	HashFunction hash_function;
} BloomFilter;



/*
	Initialize a standard bloom filter in memory; this will provide 'optimal' size and hash numbers.

	Estimated elements is 0 < x <= UINT64_MAX.
	False positive rate is 0.0 < x < 1.0

*/
int bloom_filter_init(BloomFilter *bf, uint64_t estimated_elements, float false_positive_rate, HashFunction hash_function);

/* NOT IMPLEMENTED: Import a previously exported bloom filter from a file */
int bloom_filter_import(BloomFilter *bf, char *filepath, HashFunction hash_function); // not implemented

/* NOT IMPLEMENTED: Export the current bloom filter to file */
int bloom_filter_export(BloomFilter *bf, char *filepath); // not implemented

/* Set or change the hashing function */
void bloom_filter_set_hash_function(BloomFilter *bf, HashFunction hf);

/* Print out statistics about the bloom filter */
void bloom_filter_stats(BloomFilter *bf);

/* Release all memory used by the bloom filter */
int bloom_filter_destroy(BloomFilter *bf);

/* Add a string (or element) to the bloom filter */
int bloom_filter_add_string(BloomFilter *bf, char *str);

/* Check to see if a string is or is not in the bloom filter */
int bloom_filter_check_string(BloomFilter *bf, char *str);

/* Calculates the current false positive rate based on the number of inserted elements */
float bloom_filter_current_false_positive_rate(BloomFilter *bf);


#endif /* END BLOOM FILTER HEADER */
