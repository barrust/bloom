/*******************************************************************************
***
***	 Author: Tyler Barrus
***	 email:  barrust@gmail.com
***
***	 Version: 1.6.3
***
***	 License: MIT 2015
***
*******************************************************************************/
#include "bloom.h"

//#define set_bit(A,k)	 (A[((k) / 8)] |=  (1 << ((k) % 8)))
#define clear_bit(A,k)   (A[((k) / 8)] &= ~(1 << ((k) % 8))) /* not currently used */
#define check_bit(A,k)   (A[((k) / 8)] &   (1 << ((k) % 8)))

#if defined (_OPENMP)
#define ATOMIC _Pragma ("omp atomic")
#define CRITICAL _Pragma ("omp critical")
#else
#define ATOMIC
#define CRITICAL
#endif

#define CHAR_LEN 8

static const double LOG_TWO_SQUARED = 0.4804530139182;

/*******************************************************************************
***		PRIVATE FUNCTIONS
*******************************************************************************/
static uint64_t* md5_hash_default(int num_hashes, char *str);
static void calculate_optimal_hashes(BloomFilter *bf);
static void read_from_file(BloomFilter *bf, FILE *fp, short on_disk, char *filename);
static void write_to_file(BloomFilter *bf, FILE *fp, short on_disk);
static int check_hashes(BloomFilter *bf, uint64_t *hashes, unsigned int number_hashes_passed);
static int add_hashes(BloomFilter *bf, uint64_t *hashes, unsigned int number_hashes_passed);

/*******************************************************************************
***		testing functions
*******************************************************************************/
void print_bits(char ch) {
	int i;
	for (i = 0; i < CHAR_LEN; i++) {
		printf("%c", (ch & (1 << i)) ? '1' : '0');
	}
	printf("\n");
}
/* END TESTING */

int bloom_filter_init(BloomFilter *bf, uint64_t estimated_elements, float false_positive_rate, HashFunction hash_function) {
	if(estimated_elements <= 0 || estimated_elements > UINT64_MAX) {
		return BLOOM_FAILURE;
	}
	if (false_positive_rate <= 0.0 || false_positive_rate >= 1.0 ) {
		return BLOOM_FAILURE;
	}
	bf->estimated_elements = estimated_elements;
	bf->false_positive_probability = false_positive_rate;
	calculate_optimal_hashes(bf);
	bf->bloom = calloc(bf->bloom_length, sizeof(char));
	bf->elements_added = 0;
	bloom_filter_set_hash_function(bf, hash_function);
	bf->__is_on_disk = 0; // not on disk
	return BLOOM_SUCCESS;
}

int bloom_filter_init_on_disk(BloomFilter *bf, uint64_t estimated_elements, float false_positive_rate, char *filepath, HashFunction hash_function) {
	if(estimated_elements <= 0 || estimated_elements > UINT64_MAX) {
		return BLOOM_FAILURE;
	}
	if (false_positive_rate <= 0.0 || false_positive_rate >= 1.0 ) {
		return BLOOM_FAILURE;
	}
	bf->estimated_elements = estimated_elements;
	bf->false_positive_probability = false_positive_rate;
	calculate_optimal_hashes(bf);
	bf->elements_added = 0;
	FILE *fp;
	fp = fopen(filepath, "w+b");
	if (fp == NULL) {
		fprintf(stderr, "Can't open file %s!\n", filepath);
		return BLOOM_FAILURE;
	}
	write_to_file(bf, fp, 1);
	fclose(fp);
	// slightly ineffecient to redo some of the calculations...
	return bloom_filter_import_on_disk(bf, filepath, hash_function);
}

void bloom_filter_set_hash_function(BloomFilter *bf, HashFunction hash_function) {
	bf->hash_function = (hash_function == NULL) ? md5_hash_default : hash_function;
}

int bloom_filter_destroy(BloomFilter *bf) {
	if (bf->__is_on_disk == 0) {
		free(bf->bloom);
	} else {
		fclose(bf->filepointer);
		munmap(bf->bloom, bf->__filesize);
	}
	bf->bloom = NULL;
	bf->filepointer = NULL;
	bf->elements_added = 0;
	bf->estimated_elements = 0;
	bf->false_positive_probability = 0;
	bf->number_hashes = 0;
	bf->number_bits = 0;
	bf->hash_function = NULL;
	bf->__is_on_disk = 0;
	bf->__filesize = 0;
	return BLOOM_SUCCESS;
}

void bloom_filter_stats(BloomFilter *bf) {
	char *is_on_disk = (bf->__is_on_disk == 0 ? "no" : "yes");

	printf("BloomFilter\n\
	bits: %" PRIu64 "\n\
	estimated elements: %" PRIu64 "\n\
	number hashes: %d\n\
	max false positive rate: %f\n\
	bloom length (8 bits): %ld\n\
	elements added: %" PRIu64 "\n\
	current false positive rate: %f\n\
	is on disk: %s\n",
	bf->number_bits, bf->estimated_elements, bf->number_hashes,
	bf->false_positive_probability, bf->bloom_length, bf->elements_added,
	bloom_filter_current_false_positive_rate(bf), is_on_disk);
}

int bloom_filter_add_string(BloomFilter *bf, char *str) {
	uint64_t *hashes = bloom_filter_calculate_hashes(bf, str, bf->number_hashes);
	int res = add_hashes(bf, hashes, bf->number_hashes);
	free(hashes);
	return res;
}


int bloom_filter_check_string(BloomFilter *bf, char *str) {
	uint64_t *hashes = bloom_filter_calculate_hashes(bf, str, bf->number_hashes);
	int res = check_hashes(bf, hashes, bf->number_hashes);
	free(hashes);
	return res;
}

uint64_t* bloom_filter_calculate_hashes(BloomFilter *bf, char *str, unsigned int number_hashes) {
	return bf->hash_function(number_hashes, str);
}

/* Add a string to a bloom filter using the defined hashes */
int bloom_filter_add_string_alt(BloomFilter *bf, uint64_t *hashes, unsigned int number_hashes_passed) {
	return add_hashes(bf, hashes, number_hashes_passed);
}

/* Check if a string is in the bloom filter using the passed hashes */
int bloom_filter_check_string_alt(BloomFilter *bf, uint64_t *hashes, unsigned int number_hashes_passed) {
	return check_hashes(bf, hashes, number_hashes_passed);
}

float bloom_filter_current_false_positive_rate(BloomFilter *bf) {
	int num = (bf->number_hashes * -1 * bf->elements_added);
	double d = (num * 1.0) / (bf->number_bits * 1.0);
	double e = exp(d);
	return pow((1 - e), bf->number_hashes);
}

int bloom_filter_export(BloomFilter *bf, char *filepath) {
	// if the bloom is initialized on disk, no need to export it
	if(bf->__is_on_disk == 1) {
		return BLOOM_SUCCESS;
	}
	FILE *fp;
	fp = fopen(filepath, "w+b");
	if (fp == NULL) {
		fprintf(stderr, "Can't open file %s!\n", filepath);
		return BLOOM_FAILURE;
	}
	write_to_file(bf, fp, 0);
	fclose(fp);
	return BLOOM_SUCCESS;
}

int bloom_filter_import(BloomFilter *bf, char *filepath, HashFunction hash_function) {
	FILE *fp;
	fp = fopen(filepath, "r+b");
	if (fp == NULL) {
		fprintf(stderr, "Can't open file %s!\n", filepath);
		return BLOOM_FAILURE;
	}
	read_from_file(bf, fp, 0, NULL);
	fclose(fp);
	bloom_filter_set_hash_function(bf, hash_function);
	bf->__is_on_disk = 0; // not on disk
	return BLOOM_SUCCESS;
}

int bloom_filter_import_on_disk(BloomFilter *bf, char *filepath, HashFunction hash_function) {
	bf->filepointer = fopen(filepath, "r+b");
	if (bf->filepointer == NULL) {
		fprintf(stderr, "Can't open file %s!\n", filepath);
		return BLOOM_FAILURE;
	}
	read_from_file(bf, bf->filepointer, 1, filepath);
	// don't close the file pointer here...
	bloom_filter_set_hash_function(bf, hash_function);
	bf->__is_on_disk = 1; // on disk
	return BLOOM_SUCCESS;
}

/*******************************************************************************
*	PRIVATE FUNCTIONS
*******************************************************************************/
static void calculate_optimal_hashes(BloomFilter *bf) {
	// calc optimized values
	long n = bf->estimated_elements;
	float p = bf->false_positive_probability;
	uint64_t m = ceil((-n * log(p)) / LOG_TWO_SQUARED);  // AKA pow(log(2), 2);
	unsigned int k = round(log(2.0) * m / n);
	// set paramenters
	bf->number_hashes = k; // should check to make sure it is at least 1...
	bf->number_bits = m;
	long num_pos = ceil(m / (CHAR_LEN * 1.0));
	bf->bloom_length = num_pos;
}

static int check_hashes(BloomFilter *bf, uint64_t *hashes, unsigned int number_hashes_passed) {
	if (number_hashes_passed < bf->number_hashes) {
		printf("Error: not enough hashes passed in to correctly check!\n");
		return BLOOM_FAILURE;
	}

	int r = BLOOM_SUCCESS;
	int i;
	for (i = 0; i < bf->number_hashes; i++) {
		int tmp_check = check_bit(bf->bloom, (hashes[i] % bf->number_bits));
		if (tmp_check == 0) {
			r = BLOOM_FAILURE;
			break; // no need to continue checking
		}
	}
	return r;
}

static int add_hashes(BloomFilter *bf, uint64_t *hashes, unsigned int number_hashes_passed) {
	if (number_hashes_passed < bf->number_hashes) {
		printf("Error: not enough hashes passed in to correctly check!\n");
		return BLOOM_FAILURE;
	}

	int i;
	for (i = 0; i < bf->number_hashes; i++) {
		//set_bit(bf->bloom, hashes[i]);
		ATOMIC
		bf->bloom[(hashes[i] % bf->number_bits) / 8] |=  (1 << ((hashes[i] % bf->number_bits) % 8));
	}

	ATOMIC
	bf->elements_added++;
	if(bf->__is_on_disk == 1) { // only do this if it is on disk!
		int offset = sizeof(uint64_t) + sizeof(float);
		CRITICAL
		{
			fseek(bf->filepointer, offset * -1, SEEK_END);
			fwrite(&bf->elements_added, sizeof(uint64_t), 1, bf->filepointer);
		}
	}
	return BLOOM_SUCCESS;
}

/* NOTE: this assumes that the file handler is open and ready to use */
static void write_to_file(BloomFilter *bf, FILE *fp, short on_disk) {
	if (on_disk == 0) {
		fwrite(bf->bloom, bf->bloom_length, 1, fp);
	} else {
		// will need to write out everything by hand
		uint64_t i;
		int q = 0;

		//fwrite(&q, 1, bf->bloom_length, fp);
		for (i = 0; i < bf->bloom_length; i++) {
			//fwrite(&q, 1, 1, fp);
			fputc(0, fp);
		}
	}
	fwrite(&bf->estimated_elements, sizeof(uint64_t), 1, fp);
	fwrite(&bf->elements_added, sizeof(uint64_t), 1, fp);
	fwrite(&bf->false_positive_probability, sizeof(float), 1, fp);
}

/* NOTE: this assumes that the file handler is open and ready to use */
static void read_from_file(BloomFilter *bf, FILE *fp, short on_disk, char *filename) {
	int offset = sizeof(uint64_t) * 2 + sizeof(float);
	fseek(fp, offset * -1, SEEK_END);
	fread(&bf->estimated_elements, sizeof(uint64_t), 1, fp);
	fread(&bf->elements_added, sizeof(uint64_t), 1, fp);
	fread(&bf->false_positive_probability, sizeof(float), 1, fp);
	calculate_optimal_hashes(bf);
	rewind(fp);
	if(on_disk == 0) {
		bf->bloom = calloc(bf->bloom_length, sizeof(char));
		fread(bf->bloom, sizeof(char), bf->bloom_length, fp);
	} else {
		struct stat buf;
		int fd = open(filename, O_RDWR);
		if (fd < 0) {
			perror("open: ");
			exit(1);
		}
		fstat(fd, &buf);
		bf->__filesize = buf.st_size;
		bf->bloom = mmap((caddr_t)0, bf->__filesize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
		if (bf->bloom == (unsigned char*)-1) {
			perror("mmap: ");
			exit(1);
		}
		// close the file descriptor
		close(fd);
	}
}

/* NOTE: The caller will free the results */
static uint64_t* md5_hash_default(int num_hashes, char *str) {
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
		results[i] = (uint64_t) *(uint64_t *)digest % UINT64_MAX;
	}
	return results;
}
