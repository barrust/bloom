# Bloom Filter

### Version 1.9.2 (unreleased)
* Fix: `bloom_filter_current_false_positive_rate()` overflowed an `int` once the number of hashes times the elements added passed `INT_MAX`, returning a wrong (even negative) rate
* Fix: a false positive rate above about 0.7 gave 0 hashes, so every check succeeded; `bloom_filter_init()` and `bloom_filter_init_on_disk()` now fail (check the return value before using the filter)
* Fix: `bloom_filter_estimate_elements()` was undefined behavior when every bit is set; it now returns `UINT64_MAX`
* Fix: `bloom_filter_import_hex_string()` read before the start of strings shorter than the 40 character footer, and did not check the length, the footer values, or the hex characters; it now fails for those
* Fix: `bloom_filter_import_hex_string()` wrote 2 bytes into each 1 byte element, which was only correct on little-endian machines

### Version 1.9.1
* Improved use of `const` keyword
* Minor updates

### Version 1.9.0
* ***BACKWARD INCOMPATIBLE CHANGES***
    * **NOTE:** Breaks backwards compatibility with previously exported blooms using the default hash!
    * Update to the FNV_1a hash function
    * Simplified hashing at depth by using a seed value


### Version 1.8.2
* Added CPP Guards
* Ensured that on-disk blooms correctly updated `elements_added`

### Version 1.8.1
* Improved the speed of calculating the number of bits set: [more info](https://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetTable)
* cppcheck code analysis and minor improvements

### Version 1.8.0
* Removed python version - see [pyprobables](https://github.com/barrust/pyprobables)

### Version 1.7.14
* Micro optimizations

## Version 1.7.12 and 1.7.13
* Python implementation changes and improvements

### Version 1.7.11
* Remove big endian check since exported blooms are identical on these systems
* Set critical section to a named critical section
* Fix divide by zero issue for Jaccard Index on empty Bloom Filters
* In-line wrapper functions where possible

### Version 1.7.10
* Unique HashFunction type when using with related libraries

### Version 1.7.9
* Remove `#define` construct to allow functions to better be used directly in
if-statements

### Version 1.7.8
* Pad bloom array with an additional null char

### Version 1.7.7
* Added calculating union and intersection of Bloom filters
* Added calculating the Jaccard Index of two Bloom filters
* Added estimating elements bases only on number bits set

### Version 1.7.6
* Changed default hash to FNV-1A **NOTE:** Breaks backwards compatibility with
previously exported blooms using the default hash!
* Removed **-lcrypto** requirement
* Restructured project
* Combined test files

### Version 1.7.5
* Moved reused logic into a function
* Added clear bloom filter
* Pre-computed calculation of log(2) for speed
* Renamed default hash - **NOTE:** this hash function may change in the future!

### Version 1.7.1
* Added easier to use functions when using the default hashing algorithm
* Minor clean up to reduce some function calls

### Version 1.6.2
* Updated to allow for hashes to be generated, added, and checked against
similar bloom filters without having to re-hash

### Version 1.6.0
* Updated to support parallel construction of a bloom filter using OpenMP
* **NOTE:** using parallel with on disk bloom filters is about the same as
single threaded
* Update does not force parallel construction only makes it possible. See
bloom_multi_thread.c for an example

### Version 1.5.1
* Fixed an issue when importing hundreds of blooms on disk

### Version 1.5.0
* Added the ability to import a bloom on disk

### Version 1.1.0
* Changed the export / import file format for future use in importing a bloom
on disk

### Version 1.0.0
* Original version with initialize, destroy, import and export
