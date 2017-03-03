/*
<<<<<<< HEAD
	Default tests for using the default hashing algorithm
=======
    Default tests for using the default hashing algorithm
>>>>>>> master
*/

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>  /* roundf */
<<<<<<< HEAD
#include <string.h> /* memset */
=======
>>>>>>> master
#include "../src/bloom.h"


#define ELEMENTS 50000
#define FALSE_POSITIVE_RATE 0.05
#define KEY_LEN 10

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KCYN  "\x1B[36m"

<<<<<<< HEAD
/* private functions */
int check_known_values(BloomFilter *bf, int multiple);
int check_unknown_values(BloomFilter *bf, int multiple);
int check_known_values_alt(BloomFilter *bf,int f, int s);
int check_unknown_values_alt(BloomFilter *bf, int mul, int f, int s);
void success_or_failure(int res);
static uint64_t* another_hash(int num_hashes, char *str);
static uint64_t __fnv_1a(char *key);



int main(int argc, char** argv) {
	printf("Testing BloomFilter version %s\n\n", bloom_filter_get_version());
	BloomFilter bf;
	// add a few additional spaces just in case!
	// bloom_filter_init_alt(&bf, ELEMENTS, FALSE_POSITIVE_RATE, &sha256_hash);
	bloom_filter_init(&bf, ELEMENTS, FALSE_POSITIVE_RATE);
	int i, cnt;
	for (i = 0; i < ELEMENTS * 2; i+=2) {
		char key[KEY_LEN] = {0};
		sprintf(key, "%d", i);
		bloom_filter_add_string(&bf, key);
	}
	printf("Bloom Filter insertion: ");
	if (bf.false_positive_probability == (float)FALSE_POSITIVE_RATE && bf.elements_added == ELEMENTS && roundf(100 * bloom_filter_current_false_positive_rate(&bf)) / 100 <= bf.false_positive_probability) {
=======

int check_known_values(BloomFilter *bf) {
	int i, cnt = 0;
    for (i = 0; i < ELEMENTS * 2; i+=2) {
        char key[KEY_LEN] = {0};
        sprintf(key, "%d", i);
        if (bloom_filter_check_string(bf, key) == BLOOM_FAILURE) {
            cnt++;
        }
    }
	return cnt;
}

int check_unknown_values(BloomFilter *bf) {
	int i, cnt = 0;
    for (i = 1; i < ELEMENTS * 2; i+=2) {
        char key[KEY_LEN] = {0};
        sprintf(key, "%d", i);
        if (bloom_filter_check_string(bf, key) == BLOOM_SUCCESS) {
            cnt++;
        }
    }
	return cnt;
}

void success_or_failure(int res) {
	if (res == 0) {
		printf(KGRN "success!\n" KNRM);
	} else {
		printf(KRED "failure!\n" KNRM);
	}
}


int main(int argc, char** argv) {
    printf("Testing BloomFilter version %s\n\n", bloom_filter_get_version());
    BloomFilter bf;
    // add a few additional spaces just in case!
    // bloom_filter_init_alt(&bf, ELEMENTS, FALSE_POSITIVE_RATE, &sha256_hash);
    bloom_filter_init(&bf, ELEMENTS, FALSE_POSITIVE_RATE);
    int i, cnt;
    for (i = 0; i < ELEMENTS * 2; i+=2) {
        char key[KEY_LEN] = {0};
        sprintf(key, "%d", i);
        bloom_filter_add_string(&bf, key);
    }
	printf("Bloom Filter insertion: ");
    if (bf.false_positive_probability == (float)FALSE_POSITIVE_RATE && bf.elements_added == ELEMENTS && roundf(100 * bloom_filter_current_false_positive_rate(&bf)) / 100 <= bf.false_positive_probability) {
>>>>>>> master
		success_or_failure(0);
	} else {
		success_or_failure(-1);
		// TODO: add why these failed!
	}
	printf(KCYN "NOTE:" KNRM "Bloom Filter Current False Positive Rate: %f\n", bloom_filter_current_false_positive_rate(&bf));
<<<<<<< HEAD
	bloom_filter_stats(&bf);

	printf("Bloom Filter: Check known values (all should be found): ");
	cnt = check_known_values(&bf, 2);
	success_or_failure(cnt);

	printf("Bloom Filter: Check known values (all should be either not found or false positive): ");
	cnt = check_unknown_values(&bf, 2);

	if ((float)cnt / ELEMENTS <= (float) FALSE_POSITIVE_RATE) {
=======

    printf("Bloom Filter: Check known values (all should be found): ");
    cnt = check_known_values(&bf);
	success_or_failure(cnt);

    printf("Bloom Filter: Check known values (all should be either not found or false positive): ");
    cnt = check_unknown_values(&bf);

    if ((float)cnt / ELEMENTS <= (float) FALSE_POSITIVE_RATE) {
>>>>>>> master
		success_or_failure(0);
	} else {
		success_or_failure(-1);
	}
<<<<<<< HEAD
	printf(KCYN "NOTE:" KNRM " %d flagged as possible hits! Or %f%%\n", cnt, (float)cnt / ELEMENTS);
	bloom_filter_stats(&bf);

	printf("Bloom filter export: ");
	int ex_res = bloom_filter_export(&bf, "./dist/test_bloom.blm");
=======
    printf(KCYN "NOTE:" KNRM " %d flagged as possible hits! Or %f%%\n", cnt, (float)cnt / ELEMENTS);


    printf("Bloom filter export: ");
    int ex_res = bloom_filter_export(&bf, "./dist/test_bloom.blm");
>>>>>>> master
	success_or_failure(ex_res);

	printf("Clear bloom filter: ");
	bloom_filter_clear(&bf);
	assert(bf.false_positive_probability == (float)FALSE_POSITIVE_RATE);
<<<<<<< HEAD
	assert(bf.elements_added == 0);  // should be empty!
=======
    assert(bf.elements_added == 0);  // should be empty!
>>>>>>> master
	long u;
	cnt = 0;
	for(u = 0; u < bf.bloom_length; u++) {
		if(bf.bloom[u] != 0) {
			cnt++;
		}
	}
	success_or_failure(cnt);

<<<<<<< HEAD
	printf("Cleanup original Bloom Filter: ");
	bloom_filter_destroy(&bf);
	success_or_failure(0);  // there is no failure mode for destroy


	/* import in the exported bloom filter and re-run tests */
	printf("Import from file: ");
	BloomFilter bfi;
	bloom_filter_import(&bfi, "./dist/test_bloom.blm");
=======
    printf("Cleanup original Bloom Filter: ");
    bloom_filter_destroy(&bf);
    success_or_failure(0);  // there is no failure mode for destroy


    /* import in the exported bloom filter and re-run tests */
    printf("Import from file: ");
    BloomFilter bfi;
    bloom_filter_import(&bfi, "./dist/test_bloom.blm");
>>>>>>> master
	if (bfi.false_positive_probability == (float)FALSE_POSITIVE_RATE && bfi.elements_added == ELEMENTS && roundf(100 * bloom_filter_current_false_positive_rate(&bfi)) / 100 <= bfi.false_positive_probability) {
		success_or_failure(0);
	} else {
		success_or_failure(-1);
		// TODO: add why these failed!
	}

<<<<<<< HEAD
	printf("Bloom Filter Imported: Check known values (all should be found): ");
	cnt = check_known_values(&bfi, 2);
	success_or_failure(cnt);


	printf("Bloom Filter Imported: Check known values (all should be either not found or false positive): ");
	cnt = check_unknown_values(&bfi, 2);
	if ((float)cnt / ELEMENTS <= (float) FALSE_POSITIVE_RATE) {
=======
    printf("Bloom Filter Imported: Check known values (all should be found): ");
    cnt = check_known_values(&bfi);
    success_or_failure(cnt);


    printf("Bloom Filter Imported: Check known values (all should be either not found or false positive): ");
    cnt = check_unknown_values(&bfi);
    if ((float)cnt / ELEMENTS <= (float) FALSE_POSITIVE_RATE) {
>>>>>>> master
		success_or_failure(0);
	} else {
		success_or_failure(-1);
	}
<<<<<<< HEAD
	printf(KCYN "NOTE:" KNRM " %d flagged as possible hits! Or %f%%\n", cnt, (float)cnt / ELEMENTS);


	printf("Export bloom filter as hex string: ");
	char* bloom_hex = bloom_filter_export_hex_string(&bfi);
=======
    printf(KCYN "NOTE:" KNRM " %d flagged as possible hits! Or %f%%\n", cnt, (float)cnt / ELEMENTS);


    printf("Export bloom filter as hex string: ");
    char* bloom_hex = bloom_filter_export_hex_string(&bfi);
>>>>>>> master
	if (bloom_hex != NULL) {
		success_or_failure(0);
	} else {
		success_or_failure(-1);
	}

<<<<<<< HEAD
	printf("Cleanup imported Bloom Filter: ");
	bloom_filter_destroy(&bfi);
	success_or_failure(0);  // there is basically no failure mode


	printf("Bloom Filter Hex Import: ");
	BloomFilter bfh;
	bloom_filter_import_hex_string(&bfh, bloom_hex);
=======
    printf("Cleanup imported Bloom Filter: ");
    bloom_filter_destroy(&bfi);
    success_or_failure(0);  // there is basically no failure mode


    printf("Bloom Filter Hex Import: ");
    BloomFilter bfh;
    bloom_filter_import_hex_string(&bfh, bloom_hex);
>>>>>>> master
	if (bfh.false_positive_probability == (float)FALSE_POSITIVE_RATE && bfh.elements_added == ELEMENTS && roundf(100 * bloom_filter_current_false_positive_rate(&bfh)) / 100 <= bfh.false_positive_probability) {
		success_or_failure(0);
	} else {
		success_or_failure(-1);
		// TODO: add why these failed!
	}

	printf(KCYN "NOTE:" KNRM " Free bloom hex string\n");
	free(bloom_hex);


<<<<<<< HEAD
	printf("Bloom Filter Hex: Check known values (all should be found): ");
	cnt = check_known_values(&bfh, 2);
	success_or_failure(cnt);

	printf("Bloom Filter Hex: Check known values (all should be either not found or false positive): ");
	cnt = check_unknown_values(&bfh, 2);
	if ((float)cnt / ELEMENTS <= (float) FALSE_POSITIVE_RATE) {
=======
    printf("Bloom Filter Hex: Check known values (all should be found): ");
    cnt = check_known_values(&bfh);
    success_or_failure(cnt);

    printf("Bloom Filter Hex: Check known values (all should be either not found or false positive): ");
    cnt = check_unknown_values(&bfh);
    if ((float)cnt / ELEMENTS <= (float) FALSE_POSITIVE_RATE) {
>>>>>>> master
		success_or_failure(0);
	} else {
		success_or_failure(-1);
	}
<<<<<<< HEAD
	printf(KCYN "NOTE:" KNRM " %d flagged as possible hits! Or %f%%\n", cnt, (float)cnt / ELEMENTS);


	printf("Cleanup hex Bloom Filter: ");
	bloom_filter_destroy(&bfh);
	success_or_failure(0);  // there is basically no failure mode


	printf("Bloom Filter initialize On Disk: ");
	BloomFilter bfd;
	bloom_filter_import_on_disk(&bfd, "./dist/test_bloom.blm");
=======
    printf(KCYN "NOTE:" KNRM " %d flagged as possible hits! Or %f%%\n", cnt, (float)cnt / ELEMENTS);


    printf("Cleanup hex Bloom Filter: ");
    bloom_filter_destroy(&bfh);
    success_or_failure(0);  // there is basically no failure mode


    printf("Bloom Filter initialize On Disk: ");
    BloomFilter bfd;
    bloom_filter_import_on_disk(&bfd, "./dist/test_bloom.blm");
>>>>>>> master
	if (bfd.false_positive_probability == (float)FALSE_POSITIVE_RATE && bfd.elements_added == ELEMENTS && roundf(100 * bloom_filter_current_false_positive_rate(&bfd)) / 100 <= bfd.false_positive_probability) {
		success_or_failure(0);
	} else {
		success_or_failure(-1);
		// TODO: add why these failed!
	}

<<<<<<< HEAD
	printf("Bloom Filter On Disk: Check known values (all should be found): ");
	cnt = check_known_values(&bfd, 2);
	success_or_failure(cnt);

	printf("Bloom Filter On Disk: Check known values (all should be either not found or false positive): ");
	cnt = check_unknown_values(&bfd, 2);
	if ((float)cnt / ELEMENTS <= (float) FALSE_POSITIVE_RATE) {
		success_or_failure(0);
	} else {
		success_or_failure(-1);
	}
	printf(KCYN "NOTE:" KNRM " %d flagged as possible hits! Or %f%%\n", cnt, (float)cnt / ELEMENTS);

	printf("Cleanup On Disk Bloom Filter: ");
	bloom_filter_destroy(&bfd);
	success_or_failure(0);  // there is basically no failure mode


	// test union and intersection
	printf("Bloom Filter Union: \n");
	BloomFilter bf1, bf2, bf3, res;
	bloom_filter_init(&res, ELEMENTS * 2, FALSE_POSITIVE_RATE);
	bloom_filter_init(&bf1, ELEMENTS * 2, FALSE_POSITIVE_RATE);
	bloom_filter_init(&bf2, ELEMENTS * 2, FALSE_POSITIVE_RATE);
	bloom_filter_init_alt(&bf3, ELEMENTS * 2, FALSE_POSITIVE_RATE, &another_hash);
	for (i = 0; i < ELEMENTS * 2; i+=2) {
		char key[KEY_LEN] = {0};
		sprintf(key, "%d", i);
		bloom_filter_add_string(&bf1, key);
	}
	for (i = 0; i < ELEMENTS * 3; i+=3) {
		char key[KEY_LEN] = {0};
		sprintf(key, "%d", i);
		bloom_filter_add_string(&bf2, key);
	}
	bloom_filter_union(&res, &bf1, &bf2);
	printf("Bloom Filter Union: Stats\n");
	bloom_filter_stats(&res);

	printf("Bloom Filter Union: Check inserted elements: ");
	cnt = check_known_values(&res, 2);
	cnt += check_known_values(&res, 3);
	success_or_failure(cnt);

	printf("Bloom Filter Union: Check known values (all should be either not found or false positive): ");
	cnt = check_unknown_values_alt(&res, 23, 2, 3);
	if (((float)cnt / (ELEMENTS * 2)) <= (float) FALSE_POSITIVE_RATE) {
		success_or_failure(0);
	} else {
		success_or_failure(-1);
	}
	printf(KCYN "NOTE:" KNRM " %d flagged as possible hits! Or %f%%\n", cnt, (float)cnt / (ELEMENTS * 2));

	printf("Bloom Filter Union: Check set bits to without storing: ");
	if (bloom_filter_count_set_bits(&res) == bloom_filter_count_union_bits_set(&bf1, &bf2)) {
		success_or_failure(0);
	} else {
		success_or_failure(-1);
	}

	printf("Bloom Filter Union: Check invalid hash check: ");
	if (bloom_filter_count_union_bits_set(&bf1, &bf3) == BLOOM_FAILURE) {
		success_or_failure(0);
	} else {
		success_or_failure(-1);
	}

	printf("Clear Union Bloom Filter\n");
	bloom_filter_clear(&res);


	printf("Bloom Filter Intersection: \n");
	bloom_filter_intersect(&res, &bf1, &bf2);
	printf("Bloom Filter Intersection: Stats\n");
	bloom_filter_stats(&res);

	printf("Bloom Filter Intersection: Check inserted elements: ");
	cnt = check_known_values_alt(&res, 2, 3);
	success_or_failure(cnt);

	printf("Bloom Filter Intersection: Check known values (all should be either not found or false positive): ");
	cnt = check_unknown_values_alt(&res, 23, 2, 3);
	if (((float)cnt / (ELEMENTS * 2)) <= (float) FALSE_POSITIVE_RATE) {
		success_or_failure(0);
	} else {
		success_or_failure(-1);
	}
	printf(KCYN "NOTE:" KNRM " %d flagged as possible hits! Or %f%%\n", cnt, (float)cnt / (ELEMENTS * 2));

	printf("Bloom Filter Intersection: Check set bits to without storing: ");
	if (bloom_filter_count_set_bits(&res) == bloom_filter_count_intersection_bits_set(&bf1, &bf2)) {
		success_or_failure(0);
	} else {
		success_or_failure(-1);
	}

	printf("Bloom Filter Intersection: Check invalid hash check: ");
	if (bloom_filter_count_intersection_bits_set(&bf1, &bf3) == BLOOM_FAILURE) {
		success_or_failure(0);
	} else {
		success_or_failure(-1);
	}

	printf("Bloom Filter Jaccard Index:\n");
	printf("Bloom Filter Jaccard Index: Identical Bloom Filters: ");
	if (bloom_filter_jacccard_index(&bf1, &bf1) == 1.0) {
		printf("Jaccard index: %f\t", bloom_filter_jacccard_index(&bf1, &bf1));
		success_or_failure(0);
	} else {
		success_or_failure(-1);
	}

	printf("Bloom Filter Jaccard Index: Similar Bloom Filters: ");
	if (bloom_filter_jacccard_index(&bf1, &bf2) > 0.25) {
		printf("Jaccard index: %f\t", bloom_filter_jacccard_index(&bf1, &bf2));
		success_or_failure(0);
	} else {
		success_or_failure(-1);
	}

	printf("Bloom Filter Jaccard Index: Check invalid hash check: ");
	if (bloom_filter_jacccard_index(&bf1, &bf3) == BLOOM_FAILURE) {
=======
    printf("Bloom Filter On Disk: Check known values (all should be found): ");
    cnt = check_known_values(&bfd);
    success_or_failure(cnt);

    printf("Bloom Filter On Disk: Check known values (all should be either not found or false positive): ");
    cnt = check_unknown_values(&bfd);
    if ((float)cnt / ELEMENTS <= (float) FALSE_POSITIVE_RATE) {
>>>>>>> master
		success_or_failure(0);
	} else {
		success_or_failure(-1);
	}
<<<<<<< HEAD


	printf("Cleanup Intersection Bloom Filters: ");
	bloom_filter_destroy(&bf1);
	bloom_filter_destroy(&bf2);
	bloom_filter_destroy(&bf3);
	bloom_filter_destroy(&res);

	printf("\nCompleted tests!\n");
}


/* private function definitions */
int check_known_values(BloomFilter *bf, int multiple) {
	int i, cnt = 0;
	for (i = 0; i < ELEMENTS * multiple; i+=multiple) {
		char key[KEY_LEN] = {0};
		sprintf(key, "%d", i);
		if (bloom_filter_check_string(bf, key) == BLOOM_FAILURE) {
			cnt++;
		}
	}
	return cnt;
}

int check_unknown_values(BloomFilter *bf, int multiple) {
	int i, cnt = 0;
	for (i = 1; i < ELEMENTS * multiple; i+=multiple) {
		char key[KEY_LEN] = {0};
		sprintf(key, "%d", i);
		if (bloom_filter_check_string(bf, key) == BLOOM_SUCCESS) {
			cnt++;
		}
	}
	return cnt;
}

int check_known_values_alt(BloomFilter *bf,int f, int s) {
	int i, cnt = 0;
	for (i = 0; i < ELEMENTS * f; i+=f) {
		if (i % s == 0) {
			char key[KEY_LEN] = {0};
			sprintf(key, "%d", i);
			if (bloom_filter_check_string(bf, key) == BLOOM_FAILURE) {
				cnt++;
			}
		}
	}
	return cnt;
}

int check_unknown_values_alt(BloomFilter *bf, int mul, int f, int s) {
	int i, cnt = 0;
	for (i = 1; i < ELEMENTS * mul; i+=mul) {
		if (i % s != 0 || i % f != 0) {
			char key[KEY_LEN] = {0};
			sprintf(key, "%d", i);
			if (bloom_filter_check_string(bf, key) == BLOOM_SUCCESS) {
				cnt++;
			}
		}
	}
	return cnt;
}

void success_or_failure(int res) {
	if (res == 0) {
		printf(KGRN "success!\n" KNRM);
	} else {
		printf(KRED "failure!\n" KNRM);
	}
}

/* This is the same as the current defualt hash, but is used here to get a
different hash pointer!
*/
static uint64_t* another_hash(int num_hashes, char *str) {
	uint64_t *results = calloc(num_hashes, sizeof(uint64_t));
	int i;
	char *key = calloc(17, sizeof(char));  // largest value is 7FFF,FFFF,FFFF,FFFF
	for (i = 0; i < num_hashes; i++) {
		if (i == 0) {
			results[i] = __fnv_1a(str);
		} else {
			uint64_t prev = results[i-1];
			memset(key, 0, 17);
			sprintf(key, "%" PRIx64 "", prev);
			results[i] = __fnv_1a(key);
		}
	}
	free(key);
	return results;
}

static uint64_t __fnv_1a(char *key) {
	// FNV-1a hash (http://www.isthe.com/chongo/tech/comp/fnv/)
	int i, len = strlen(key);
	uint64_t h = 14695981039346656073ULL; // FNV_OFFSET 64 bit
	for (i = 0; i < len; i++){
			h = h ^ (unsigned char) key[i];
			h = h * 1099511628211ULL; // FNV_PRIME 64 bit
	}
	return h;
}
=======
    printf(KCYN "NOTE:" KNRM " %d flagged as possible hits! Or %f%%\n", cnt, (float)cnt / ELEMENTS);

    printf("Cleanup On Disk Bloom Filter: ");
    bloom_filter_destroy(&bfd);
    success_or_failure(0);  // there is basically no failure mode

	printf("\nCompleted tests!\n");
}
>>>>>>> master
