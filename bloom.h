/*******************************************************************************
***
***	 Author: Tyler Barrus
***	 email:  barrust@gmail.com
***
***	 Version: 1.7.5
***	 Purpose: Simple, yet effective, bloom filter implementation
***
***	 License: MIT 2015
***
***	 URL: https://github.com/barrust/bloom
***
***	 Usage:
***			BloomFilter bf;
***			bloom_filter_init(&bf, 100000, 0.05, NULL); // 5% false positive rate
***			bloom_filter_add_string(&bf, "google");
***			bloom_filter_add_string(&bf, "twitter");
***			if (bloom_filter_check_string(&bf, "facebook") == BLOOM_FAILURE) {
***				printf("facebook is not present!\n");
***			}
***			bloom_filter_destroy(&bf);
***
***	Required Compile Flags: -lm -lcrypto
***
*******************************************************************************/
#ifndef __BLOOM_FILTER_H__
#define __BLOOM_FILTER_H__

#include <stdlib.h>
#include <inttypes.h>       /* PRIu64 */
#include <math.h>           /* pow, exp */
#include <stdio.h>          /* printf */
#include <string.h>         /* strlen */
#include <fcntl.h>          /* O_RDWR */
#include <sys/mman.h>       /* mmap, mummap */
#include <sys/types.h>      /* */
#include <sys/stat.h>       /* fstat */
#include <unistd.h>         /* close */
#include <openssl/md5.h>

#ifdef __APPLE__
	#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#define BLOOMFILTER_VERSION "1.7.5"
#define BLOOMFILTER_MAJOR 1
#define BLOOMFILTER_MINOR 7
#define BLOOMFILTER_REVISION 5

#define BLOOM_SUCCESS 0
#define BLOOM_FAILURE -1

#define bloom_filter_get_version()	(BLOOMFILTER_VERSION)

typedef uint64_t* (*HashFunction) (int num_hashes, char *str);

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
	/* on disk handeling */
	short __is_on_disk;
	FILE *filepointer;
	uint64_t __filesize;
} BloomFilter;



/*
	Initialize a standard bloom filter in memory; this will provide 'optimal' size and hash numbers.

	Estimated elements is 0 < x <= UINT64_MAX.
	False positive rate is 0.0 < x < 1.0
*/
#define bloom_filter_init(bf, estimated_elements, false_positive_rate) {bloom_filter_init_alt(bf, estimated_elements, false_positive_rate, NULL);}
int bloom_filter_init_alt(BloomFilter *bf, uint64_t estimated_elements, float false_positive_rate, HashFunction hash_function);

/* Initialize a bloom filter directly into file; useful if the bloom filter is larger than available RAM */
#define bloom_filter_init_on_disk(bf, estimated_elements, false_positive_rate, filepath) {bloom_filter_init_on_disk_alt(bf, estimated_elements, false_positive_rate, filepath, NULL);}
int bloom_filter_init_on_disk_alt(BloomFilter *bf, uint64_t estimated_elements, float false_positive_rate, char *filepath, HashFunction hash_function);

/* Import a previously exported bloom filter from a file into memory */
#define bloom_filter_import(bf, filepath) {bloom_filter_import_alt(bf, filepath, NULL);}
int bloom_filter_import_alt(BloomFilter *bf, char *filepath, HashFunction hash_function);

/*
	Import a previously exported bloom filter from a file but do not pull the full bloom into memory.
	This is allows for the speed / storage trade off of not needing to put the full bloom filter
	into RAM.
*/
#define bloom_filter_import_on_disk(bf,filepath) {bloom_filter_import_on_disk_alt(bf, filepath, NULL);}
int bloom_filter_import_on_disk_alt(BloomFilter *bf, char *filepath, HashFunction hash_function);

/* Export the current bloom filter to file */
int bloom_filter_export(BloomFilter *bf, char *filepath);

/*
	Export and import as a hex string; not space effecient but allows for storing
	multiple blooms in a single file or in a database, etc.

	NOTE: It is up to the caller to free the allocated memory
*/
char* bloom_filter_export_hex_string(BloomFilter *bf);
#define bloom_filter_import_hex_string(bf, hex) {bloom_filter_import_hex_string_alt(bf, hex, NULL);}
int bloom_filter_import_hex_string_alt(BloomFilter *bf, char *hex, HashFunction hash_function);

/* Set or change the hashing function */
void bloom_filter_set_hash_function(BloomFilter *bf, HashFunction hash_function);

/* Print out statistics about the bloom filter */
void bloom_filter_stats(BloomFilter *bf);

/* Release all memory used by the bloom filter */
int bloom_filter_destroy(BloomFilter *bf);

/* reset filter to unused state */
int bloom_filter_clear(BloomFilter *bf);

/* Add a string (or element) to the bloom filter */
int bloom_filter_add_string(BloomFilter *bf, char *str);

/* Add a string to a bloom filter using the defined hashes */
int bloom_filter_add_string_alt(BloomFilter *bf, uint64_t *hashes, unsigned int number_hashes_passed);

/* Check to see if a string (or element) is or is not in the bloom filter */
int bloom_filter_check_string(BloomFilter *bf, char *str);

/* Check if a string is in the bloom filter using the passed hashes */
int bloom_filter_check_string_alt(BloomFilter *bf, uint64_t *hashes, unsigned int number_hashes_passed);

/* Calculates the current false positive rate based on the number of inserted elements */
float bloom_filter_current_false_positive_rate(BloomFilter *bf);

/*
	Generate the desired number of hashes for the provided string

	NOTE: It is up to the caller to free the allocated memory
*/
uint64_t* bloom_filter_calculate_hashes(BloomFilter *bf, char *str, unsigned int number_hashes);

/* Calculate the size the bloom filter will take on disk when exported */
uint64_t bloom_filter_export_size(BloomFilter *bf);

#endif /* END BLOOM FILTER HEADER */
