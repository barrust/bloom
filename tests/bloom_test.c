/*
	Default tests for using the default hashing algorithm
*/

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>  /* roundf */
#include "../src/bloom.h"


#define ELEMENTS 50000
#define FALSE_POSITIVE_RATE 0.05
#define KEY_LEN 10

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KCYN  "\x1B[36m"

/* private functions */
int check_known_values(BloomFilter *bf, int mult);
int check_known_values_alt(BloomFilter *bf, int mult, int mult2, int* used);
int check_unknown_values(BloomFilter *bf, int mult);
int check_unknown_values_alt(BloomFilter *bf, int mult, int mult2, int offset, int* used);
int check_unknown_values_alt_2(BloomFilter *bf, int mult, int mult2, int offset, int* used);
void success_or_failure(int res);
void populate_bloom_filter(BloomFilter *bf, unsigned long long elements, int mult);


int main(int argc, char** argv) {
	printf("Testing BloomFilter version %s\n\n", bloom_filter_get_version());
	BloomFilter bf;
	// add a few additional spaces just in case!
	// bloom_filter_init_alt(&bf, ELEMENTS, FALSE_POSITIVE_RATE, &sha256_hash);
	bloom_filter_init(&bf, ELEMENTS, FALSE_POSITIVE_RATE);
	int cnt, used;
	populate_bloom_filter(&bf, ELEMENTS, 2);
	printf("Bloom Filter insertion: ");
	if (bf.false_positive_probability == (float)FALSE_POSITIVE_RATE && bf.elements_added == ELEMENTS && roundf(100 * bloom_filter_current_false_positive_rate(&bf)) / 100 <= bf.false_positive_probability) {
		success_or_failure(0);
	} else {
		success_or_failure(-1);
		// TODO: add why these failed!
	}
	printf(KCYN "NOTE:" KNRM "Bloom Filter Current False Positive Rate: %f\n", bloom_filter_current_false_positive_rate(&bf));

	printf("Bloom Filter: Check known values (all should be found): ");
	cnt = check_known_values(&bf, 2);
	success_or_failure(cnt);

	printf("Bloom Filter: Check known values (all should be either not found or false positive): ");
	cnt = check_unknown_values(&bf, 2);

	if ((float)cnt / ELEMENTS <= (float) FALSE_POSITIVE_RATE) {
		success_or_failure(0);
	} else {
		success_or_failure(-1);
	}
	printf(KCYN "NOTE:" KNRM " %d flagged as possible hits! Or %f%%\n", cnt, (float)cnt / ELEMENTS);


	printf("Bloom filter export: ");
	int ex_res = bloom_filter_export(&bf, "./dist/test_bloom.blm");
	success_or_failure(ex_res);

	printf("Clear bloom filter: ");
	bloom_filter_clear(&bf);
	assert(bf.false_positive_probability == (float)FALSE_POSITIVE_RATE);
	assert(bf.elements_added == 0);  // should be empty!
	long u;
	cnt = 0;
	for(u = 0; u < bf.bloom_length; u++) {
		if(bf.bloom[u] != 0) {
			cnt++;
		}
	}
	success_or_failure(cnt);

	printf("Cleanup original Bloom Filter: ");
	bloom_filter_destroy(&bf);
	success_or_failure(0);  // there is no failure mode for destroy


	/* import in the exported bloom filter and re-run tests */
	printf("Import from file: ");
	BloomFilter bfi;
	bloom_filter_import(&bfi, "./dist/test_bloom.blm");
	if (bfi.false_positive_probability == (float)FALSE_POSITIVE_RATE && bfi.elements_added == ELEMENTS && roundf(100 * bloom_filter_current_false_positive_rate(&bfi)) / 100 <= bfi.false_positive_probability) {
		success_or_failure(0);
	} else {
		success_or_failure(-1);
		// TODO: add why these failed!
	}

	printf("Bloom Filter Imported: Check known values (all should be found): ");
	cnt = check_known_values(&bfi, 2);
	success_or_failure(cnt);


	printf("Bloom Filter Imported: Check known values (all should be either not found or false positive): ");
	cnt = check_unknown_values(&bfi, 2);
	if ((float)cnt / ELEMENTS <= (float) FALSE_POSITIVE_RATE) {
		success_or_failure(0);
	} else {
		success_or_failure(-1);
	}
	printf(KCYN "NOTE:" KNRM " %d flagged as possible hits! Or %f%%\n", cnt, (float)cnt / ELEMENTS);


	printf("Export bloom filter as hex string: ");
	char* bloom_hex = bloom_filter_export_hex_string(&bfi);
	if (bloom_hex != NULL) {
		success_or_failure(0);
	} else {
		success_or_failure(-1);
	}

	printf("Cleanup imported Bloom Filter: ");
	bloom_filter_destroy(&bfi);
	success_or_failure(0);  // there is basically no failure mode


	printf("Bloom Filter Hex Import: ");
	BloomFilter bfh;
	bloom_filter_import_hex_string(&bfh, bloom_hex);
	if (bfh.false_positive_probability == (float)FALSE_POSITIVE_RATE && bfh.elements_added == ELEMENTS && roundf(100 * bloom_filter_current_false_positive_rate(&bfh)) / 100 <= bfh.false_positive_probability) {
		success_or_failure(0);
	} else {
		success_or_failure(-1);
		// TODO: add why these failed!
	}

	printf(KCYN "NOTE:" KNRM " Free bloom hex string\n");
	free(bloom_hex);


	printf("Bloom Filter Hex: Check known values (all should be found): ");
	cnt = check_known_values(&bfh, 2);
	success_or_failure(cnt);

	printf("Bloom Filter Hex: Check known values (all should be either not found or false positive): ");
	cnt = check_unknown_values(&bfh, 2);
	if ((float)cnt / ELEMENTS <= (float) FALSE_POSITIVE_RATE) {
		success_or_failure(0);
	} else {
		success_or_failure(-1);
	}
	printf(KCYN "NOTE:" KNRM " %d flagged as possible hits! Or %f%%\n", cnt, (float)cnt / ELEMENTS);


	printf("Cleanup hex Bloom Filter: ");
	bloom_filter_destroy(&bfh);
	success_or_failure(0);  // there is basically no failure mode


	printf("Bloom Filter initialize On Disk: ");
	BloomFilter bfd;
	bloom_filter_import_on_disk(&bfd, "./dist/test_bloom.blm");
	if (bfd.false_positive_probability == (float)FALSE_POSITIVE_RATE && bfd.elements_added == ELEMENTS && roundf(100 * bloom_filter_current_false_positive_rate(&bfd)) / 100 <= bfd.false_positive_probability) {
		success_or_failure(0);
	} else {
		success_or_failure(-1);
		// TODO: add why these failed!
	}

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



	BloomFilter res;
	BloomFilter bf1;
	BloomFilter bf2;
	printf("Bloom Filter Union / Intersection / Jaccard Index: setup Bloom Filters: ");
	bloom_filter_init(&res, ELEMENTS * 4, FALSE_POSITIVE_RATE);
	bloom_filter_init(&bf1, ELEMENTS * 4, FALSE_POSITIVE_RATE);
	bloom_filter_init(&bf2, ELEMENTS * 4, FALSE_POSITIVE_RATE);

	populate_bloom_filter(&bf1, ELEMENTS * 2, 2);
	populate_bloom_filter(&bf2, ELEMENTS * 2, 3);
	cnt = check_known_values(&bf1, 2);
	cnt += check_known_values(&bf2, 3);
	success_or_failure(cnt);

	printf("Bloom Filter Union: \n");
	printf("Bloom Filter Union: known values: ");
	bloom_filter_union(&res, &bf1, &bf2);
	cnt = check_known_values(&res, 2);
	cnt += check_known_values(&res, 3);
	success_or_failure(cnt);

	printf("Bloom Filter Union: unknown values: ");
	cnt = check_unknown_values_alt(&res, 2, 3, 11, &used);
	if ((float)cnt / used <= (float) FALSE_POSITIVE_RATE) {
		success_or_failure(0);
	} else {
		success_or_failure(-1);
	}
	printf(KCYN "NOTE:" KNRM " %d flagged as possible hits out of %d elements! Or %f%%\n", cnt, used, (float)cnt / used);
	bloom_filter_stats(&res);

	printf("Bloom Filter Union: count set bits without storing: ");
	if (bloom_filter_count_union_bits_set(&bf1, &bf2) == bloom_filter_count_set_bits(&res)) {
		success_or_failure(0);
	} else {
		success_or_failure(-1);
	}

	bloom_filter_clear(&res);
	printf("Bloom Filter Intersection: \n");
	printf("Bloom Filter Intersection: known values: ");
	bloom_filter_clear(&res);
	bloom_filter_intersect(&res, &bf1, &bf2);
	cnt = check_known_values_alt(&res, 2, 3, &used);
	success_or_failure(cnt);

	printf("Bloom Filter Union: unknown values: ");
	cnt = check_unknown_values_alt_2(&res, 2, 3, 23, &used);
	if ((float)cnt / used <= (float) FALSE_POSITIVE_RATE) {
		success_or_failure(0);
	} else {
		success_or_failure(-1);
	}
	printf(KCYN "NOTE:" KNRM " %d flagged as possible hits out of %d elements! Or %f%%\n", cnt, used, (float)cnt / used);
	bloom_filter_stats(&res);

	printf("Bloom Filter Intersection: count set bits without storing: ");
	if (bloom_filter_count_intersection_bits_set(&bf1, &bf2) == bloom_filter_count_set_bits(&res)) {
		success_or_failure(0);
	} else {
		success_or_failure(-1);
	}

	printf("Bloom Filter Jaccard Index: \n");
	printf("Bloom Filter Jaccard Index: same bloom: ");
	if (bloom_filter_jacccard_index(&res, &res) == 1) {
		success_or_failure(0);
	} else {
		success_or_failure(-1);
	}
	
	printf("Bloom Filter Jaccard Index: ~30 percent similar bloom: ");
	if (bloom_filter_jacccard_index(&bf1, &bf2) < .35) {
		success_or_failure(0);
	} else {
		success_or_failure(-1);
	}
	printf(KCYN "NOTE:" KNRM " similarity score: %f\n", bloom_filter_jacccard_index(&bf1, &bf2));




	bloom_filter_destroy(&res);
	bloom_filter_destroy(&bf1);
	bloom_filter_destroy(&bf2);

	printf("\nCompleted tests!\n");
}


/* private function definitions */
void populate_bloom_filter(BloomFilter *bf, unsigned long long elements, int mult) {
	int i;
	for (i = 0; i < elements * mult; i+=mult) {
		char key[KEY_LEN] = {0};
		sprintf(key, "%d", i);
		bloom_filter_add_string(bf, key);
	}
}

int check_known_values(BloomFilter *bf, int mult) {
	int i, cnt = 0;
	for (i = 0; i < ELEMENTS * mult; i+=mult) {
		char key[KEY_LEN] = {0};
		sprintf(key, "%d", i);
		if (bloom_filter_check_string(bf, key) == BLOOM_FAILURE) {
			cnt++;
		}
	}
	return cnt;
}

int check_known_values_alt(BloomFilter *bf, int mult, int mult2, int* used) {
	int i, cnt = 0;
	int j = 0;
	for (i = 0; i < ELEMENTS * mult; i+=mult) {
		if (i % mult2 == 0 && i % mult == 0) {
			char key[KEY_LEN] = {0};
			sprintf(key, "%d", i);
			if (bloom_filter_check_string(bf, key) == BLOOM_FAILURE) {
				cnt++;
			}
			j++;
		}
	}
	*used = j;
	return cnt;
}

int check_unknown_values(BloomFilter *bf, int mult) {
	int i, cnt = 0;
	for (i = 1; i < ELEMENTS * mult; i+=mult) {
		char key[KEY_LEN] = {0};
		sprintf(key, "%d", i);
		if (bloom_filter_check_string(bf, key) == BLOOM_SUCCESS) {
			cnt++;
		}
	}
	return cnt;
}

int check_unknown_values_alt(BloomFilter *bf, int mult, int mult2, int offset, int* used) {
	int i, cnt = 0;
	int j = 0;
	for (i = offset; i < ELEMENTS * offset; i+=offset) {
		if (i % mult2 == 0 || i % mult == 0) {
			// pass
		} else {
			char key[KEY_LEN] = {0};
			sprintf(key, "%d", i);
			if (bloom_filter_check_string(bf, key) == BLOOM_SUCCESS) {
				cnt++;
			}
			j++;
		}
	}
	*used = j;
	return cnt;
}

int check_unknown_values_alt_2(BloomFilter *bf, int mult, int mult2, int offset, int* used) {
	int i, cnt = 0;
	int j = 0;
	for (i = offset; i < ELEMENTS * offset; i+=offset) {
		if (i % mult2 == 0 && i % mult == 0) {
			// pass
		} else {
			char key[KEY_LEN] = {0};
			sprintf(key, "%d", i);
			if (bloom_filter_check_string(bf, key) == BLOOM_SUCCESS) {
				cnt++;
			}
			j++;
		}
	}
	*used = j;
	return cnt;
}

void success_or_failure(int res) {
	if (res == 0) {
		printf(KGRN "success!\n" KNRM);
	} else {
		printf(KRED "failure!\n" KNRM);
	}
}
