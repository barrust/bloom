###Version 1.7.6
* Changed default hash to FNV-1A **NOTE:** Breaks backwards compatibility with previously exported blooms with default hash!
* Removed **-lcrypto** requirement

###Version 1.7.5
* Moved reused logic into a function
* Added clear bloom filter
* Pre-computed calculation of log(2) for speed
* Renamed default hash - **NOTE:** this hash function may change in the future!

###Version 1.7.1
* Added easier to use functions when using the default hashing algorithm
* Minor clean up to reduce some function calls

###Version 1.6.2
* Updated to allow for hashes to be generated, added, and checked against similar bloom filters without having to re-hash

###Version 1.6.0
* Updated to support parallel construction of a bloom filter using OpenMP
* NOTE: using parallel with on disk bloom filters is about the same as single threaded
* Update does not force parallel construction only makes it possible. See bloom_multi_thread.c for an example

###Version 1.5.1
* Fixed an issue when importing hundreds of blooms on disk

###Version 1.5.0
* Added the ability to import a bloom on disk

###Version 1.1.0
* Changed the export / import file format for future use in importing a bloom on disk

###Version 1.0.0
* Original version with initialize, destroy, import and export
