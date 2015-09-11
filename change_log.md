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
