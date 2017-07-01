''' BloomFilter, python implementation
    License: MIT
    Author: Tyler Barrus (barrust@gmail.com)
    URL: https://github.com/barrust/bloom
'''
from __future__ import (print_function)
import math
import struct
import os
import mmap
from binascii import (hexlify, unhexlify)


class BloomFilter(object):
    ''' Simple Bloom Filter implementation for use in python;
    It can read and write the same format as the c version

    NOTE: Does *not* support the 'on disk' opperations!'''

    def __init__(self):
        ''' setup the basic values needed '''
        self._bloom = None
        self.__num_bits = 0  # number of bits
        self._est_elements = 0
        self._fpr = 0.0
        self.__number_hashes = 0
        self.__bloom_length = self.number_bits / 8
        self.__hash_func = self._default_hash
        self.__els_added = 0
        self._on_disk = False  # not on disk

    @property
    def bloom_array(self):
        ''' access to the bloom array itself '''
        return self._bloom

    @property
    def false_positive_rate(self):
        ''' desired max false positive rate '''
        return self._fpr

    @property
    def estimated_elements(self):
        ''' the number of elements estimated to be added when setup '''
        return self._est_elements

    @property
    def number_hashes(self):
        ''' the number of hashes for the bloom filter '''
        return self.__number_hashes

    @number_hashes.setter
    def number_hashes(self, value):
        self.__number_hashes = value

    @property
    def number_bits(self):
        ''' number of bits used '''
        return self.__num_bits

    @property
    def elements_added(self):
        ''' get the number of elements added '''
        return self.__els_added

    @elements_added.setter
    def elements_added(self, value):
        ''' set the number of elements added '''
        self.__els_added = value

    @property
    def is_on_disk(self):
        ''' get the number of elements added '''
        return self._on_disk

    @property
    def bloom_length(self):
        return self.__bloom_length

    def __str__(self):
        ''' correctly handle python 3 vs python2 encoding if necessary '''
        return self.__unicode__()

    def __unicode__(self):
        ''' string / unicode representation of the bloom filter '''
        on_disk = "no" if self.is_on_disk is False else "yes"
        stats = ('BloomFilter: \n'
                 '\tbits: {0}\n'
                 '\testimated elements: {1}\n'
                 '\tnumber hashes: {2}\n'
                 '\tmax false positive rate: {3:.6f}\n'
                 '\tbloom length (8 bits): {4}\n'
                 '\telements added: {5}\n'
                 '\testimated elements added: {6}\n'
                 '\tcurrent false positive rate: {7:.6f}\n'
                 '\texport size (bytes): {8}\n'
                 '\tnumber bits set: {9}\n'
                 '\tis on disk: {10}\n')
        return stats.format(self.number_bits, self.estimated_elements,
                            self.number_hashes, self.false_positive_rate,
                            self.bloom_length, self.elements_added,
                            self.estimate_elements(),
                            self.current_false_positive_rate(),
                            self.export_size(), self._cnt_number_bits_set(),
                            on_disk)

    def init(self, est_elements, false_positive_rate, hash_function=None):
        ''' initialize the bloom filter '''
        self._set_optimized_params(est_elements, false_positive_rate, 0,
                                hash_function)
        self._bloom = [0] * self.bloom_length

    def hashes(self, key, depth=None):
        ''' calculate the hashes for the passed in key '''
        tmp = depth if depth is not None else self.number_hashes
        return self.__hash_func(key, tmp)

    def add(self, key):
        ''' add the key to the bloom filter '''
        hashes = self.hashes(key)
        self.add_alt(hashes)

    def add_alt(self, hashes):
        ''' add the element represented by hashes into the bloom filter '''
        for i in list(range(0, self.number_hashes)):
            k = int(hashes[i]) % self.number_bits
            idx = k // 8
            self.bloom_array[idx] = int(self.bloom_array[idx]) | int((1 << (k % 8)))
        self.elements_added += 1

    def check(self, key):
        ''' check if the key is likely in the bloom filter '''
        hashes = self.hashes(key)
        return self.check_alt(hashes)

    def check_alt(self, hashes):
        ''' check if the element represented by hashes is in the bloom filter
        '''
        for i in list(range(0, self.number_hashes)):
            k = int(hashes[i]) % self.number_bits
            if (int(self.bloom_array[k // 8]) & int((1 << (k % 8)))) == 0:
                return False
        return True

    def intersection(self, second):
        ''' return a new Bloom Filter that contains the intersection of the two
        '''
        if self.__verify_bloom_similarity(second) is False:
            return None
        res = BloomFilter()
        res.init(self.estimated_elements, self.false_positive_rate,
                 self.__hash_func)

        for i in list(range(0, self.bloom_length)):
            res.bloom_array[i] = self.bloom_array[i] & second.bloom_array[i]
        res.elements_added = res.estimate_elements()
        return res

    def union(self, second):
        ''' return a new Bloom Filter that contains the union of the two '''
        if self.__verify_bloom_similarity(second) is False:
            return None
        res = BloomFilter()
        res.init(self.estimated_elements, self.false_positive_rate,
                 self.__hash_func)

        for i in list(range(0, self.bloom_length)):
            res.bloom_array[i] = self.bloom_array[i] | second.bloom_array[i]
        res.elements_added = res.estimate_elements()
        return res

    def jaccard_index(self, second):
        ''' calculate the jaccard similarity score '''
        if self.__verify_bloom_similarity(second) is False:
            return None
        count_union = 0
        count_int = 0
        for i in list(range(0, self.bloom_length)):
            t_union = self.bloom_array[i] | second.bloom_array[i]
            t_intersection = self.bloom_array[i] & second.bloom_array[i]
            count_union += self._cnt_set_bits(t_union)
            count_int += self._cnt_set_bits(t_intersection)
        if count_union == 0:
            return 1.0
        return float(count_int) / float(count_union)

    def export(self, filename):
        ''' export the bloom filter to disk '''
        with open(filename, 'wb') as filepointer:
            rep = 'B' * self.bloom_length
            filepointer.write(struct.pack(rep, *self.bloom_array))
            filepointer.write(struct.pack('QQf', self.estimated_elements,
                                          self.elements_added,
                                          self.false_positive_rate))

    def load(self, filename, hash_function=None):
        ''' load the bloom filter from file '''
        # read in the needed information, and then call _set_optimized_params
        # to set everything correctly
        with open(filename, 'rb') as filepointer:
            offset = struct.calcsize('QQf')
            filepointer.seek(offset * -1, os.SEEK_END)
            mybytes = struct.unpack('QQf', filepointer.read(offset))
            self._est_elements = mybytes[0]
            self.elements_added = mybytes[1]
            self._fpr = mybytes[2]

            self._set_optimized_params(self.estimated_elements,
                                    self.false_positive_rate,
                                    self.elements_added, hash_function)

            # now read in the bit array!
            filepointer.seek(0, os.SEEK_SET)
            offset = struct.calcsize('B') * self.bloom_length
            rep = 'B' * self.bloom_length
            self._bloom = list(struct.unpack(rep, filepointer.read(offset)))

    def export_hex(self):
        ''' export Bloom Filter to hex string '''
        mybytes = struct.pack('>QQf', self.estimated_elements,
                              self.elements_added, self.false_positive_rate)
        return hexlify(bytearray(self.bloom_array)) + hexlify(mybytes)

    def load_hex(self, hex_string, hash_function=None):
        ''' placeholder for loading from hex string '''
        offset = struct.calcsize('>QQf') * 2
        stct = struct.Struct('>QQf')
        tmp_data = stct.unpack_from(unhexlify(hex_string[-offset:]))
        self._set_optimized_params(tmp_data[0], tmp_data[2], tmp_data[1],
                                hash_function)
        tmp_bloom = unhexlify(hex_string[:-offset])
        rep = 'B' * self.bloom_length
        self._bloom = list(struct.unpack(rep, tmp_bloom))

    def export_size(self):
        ''' calculate the size of the bloom on disk '''
        tmp_b = struct.calcsize('B')
        return (self.bloom_length * tmp_b) + struct.calcsize('QQf')

    def estimate_elements(self):
        ''' estimate the number of elements added '''
        setbits = self._cnt_number_bits_set()
        log_n = math.log(1 - (float(setbits) / float(self.number_bits)))
        tmp = float(self.number_bits) / float(self.number_hashes)
        return int(-1 * tmp * log_n)

    def current_false_positive_rate(self):
        ''' calculate the current false positive rate '''
        num = self.number_hashes * -1 * self.elements_added
        dbl = num / float(self.number_bits)
        exp = math.exp(dbl)
        return math.pow((1 - exp), self.number_hashes)

    def _set_optimized_params(self, estimated_elements, false_positive_rate,
                           elements_added, hash_function):
        ''' set the parameters to the optimal sizes '''
        if hash_function is None:
            self.__hash_func = self._default_hash
        else:
            self.__hash_func = hash_function
        self._est_elements = estimated_elements
        fpr = struct.pack('f', float(false_positive_rate))
        self._fpr = struct.unpack('f', fpr)[0]  # to mimic the c version!
        self.elements_added = elements_added
        # optimal caluclations
        n_els = self.estimated_elements
        fpr = float(self._fpr)
        m_bt = math.ceil((-n_els * math.log(fpr)) / 0.4804530139182)  # ln(2)^2
        self.number_hashes = int(round(math.log(2.0) * m_bt / n_els))
        self.__num_bits = int(m_bt)
        self.__bloom_length = int(math.ceil(m_bt / (8 * 1.0)))

    def __verify_bloom_similarity(self, second):
        ''' can the blooms be used in intersection, union, or jaccard index '''
        hash_match = self.number_hashes != second.number_hashes
        same_bits = self.number_bits != second.number_bits
        next_hash = self.hashes("test") != second.hashes("test")
        if hash_match or same_bits or next_hash:
            return False
        return True

    @staticmethod
    def _cnt_set_bits(i):
        ''' count number of bits set '''
        return bin(i).count("1")

    def _cnt_number_bits_set(self):
        ''' calculate the total number of set bits in the bloom '''
        setbits = 0
        for i in list(range(0, self.bloom_length)):
            setbits += self._cnt_set_bits(self.bloom_array[i])
        return setbits

    def _default_hash(self, key, depth):
        ''' the default fnv-1a hashing routine '''
        res = list()
        tmp = key
        for _ in list(range(0, depth)):
            if tmp != key:
                tmp = self.__fnv_1a("{0:x}".format(tmp))
            else:
                tmp = self.__fnv_1a(key)
            res.append(tmp)
        return res

    @staticmethod
    def __fnv_1a(key):
        ''' 64 bit fnv-1a hash '''
        hval = 14695981039346656073
        fnv_64_prime = 1099511628211
        uint64_max = 2 ** 64
        for tmp_s in key:
            hval = hval ^ ord(tmp_s)
            hval = (hval * fnv_64_prime) % uint64_max
        return hval


class BloomFilterOnDisk(BloomFilter):
    def __init__(self):
        super(BloomFilterOnDisk, self).__init__()

    def initialize(self, filepath, est_elements, false_positive_rate, hash_function=None):
        ''' initialize the Bloom Filter on disk '''
        fpr = false_positive_rate
        super(BloomFilterOnDisk, self)._set_optimized_params(est_elements,
                                                             fpr, 0,
                                                             hash_function)
        # do the on disk things
        with open(filepath, 'wb') as filepointer:
            for i in range(self.bloom_length):
                filepointer.write(struct.pack('B', int(0)))
            filepointer.write(struct.pack('QQf', est_elements,
                                          0,
                                          false_positive_rate))
            filepointer.flush()
        self.load(filepath, hash_function)

    def close(self):
        ''' clean up the memory '''
        self.__file_pointer.close()


    def load(self, filepath, hash_function=None):
        ''' load the bloom filter on disk '''
        # read the file, set the optimal params
        # mmap everything
        with open(filepath, 'r+b') as filepointer:
            offset = struct.calcsize('QQf')
            filepointer.seek(offset * -1, os.SEEK_END)
            mybytes = struct.unpack('QQf', filepointer.read(offset))
            self._est_elements = mybytes[0]
            self.elements_added = mybytes[1]
            self._fpr = mybytes[2]
            self._set_optimized_params(mybytes[0], mybytes[2], mybytes[1], hash_function)
        self.__file_pointer = open(filepath, 'r+b')
        self._bloom = mmap.mmap(self.__file_pointer.fileno(), 0)
        self._on_disk = True
        self.__filename = filepath

    def export(self, filename):
        ''' export to disk if a different location '''
        if filename != self.__filename:
            super(BloomFilterOnDisk, self).export(filename)
        # otherwise, nothing to do!

    def add_alt(self, hashes):
        ''' add the element represented by the hashes to the Bloom Filter on disk '''
        for x in range(self.number_hashes):
            k = long(hashes[x]) % self.number_bits
            idx = k / 8
            c = struct.unpack('B', self.bloom_array[idx])[0]
            self.bloom_array[idx] = struct.pack('B', int(c) | int((1 << (k % 8))))
            self.bloom_array.flush()
        self.elements_added += 1
        self.__file_pointer.seek(-12, os.SEEK_END)
        self.__file_pointer.write(struct.pack('Q', self.elements_added))
        self.bloom_array.flush()
        self.__file_pointer.flush() # make sure everything is out to disk

    def check_alt(self, hashes):
        ''' check if the hashes relate '''
        for i in range(0, self.number_hashes):
            k = long(hashes[i]) % self.number_bits
            c = struct.unpack('B', self.bloom_array[k / 8])[0]
            if (c & int(1 << (k % 8))) == 0:
                return False
        return True

    def _cnt_number_bits_set(self):
        ''' override this to handle the on disk mmap '''
        setbits = 0
        for i in list(range(0, self.bloom_length)):
            tmp_bin = struct.unpack('B', self.bloom_array[i])[0]
            setbits += self._cnt_set_bits(tmp_bin)
        return setbits

if __name__ == '__main__':
    blm = BloomFilter()
    blm.init(10, 0.05)
    blm.add("this is a test")
    print(blm.check("this is a test"))
    print(blm.check("blah"))
    print(blm)
    print(blm.bloom_array)
    blm.export('./dist/py_bloom.blm')

    print('\n\ncheck imported BloomFilter!')

    blm2 = BloomFilter()
    blm2.load('./dist/py_bloom.blm')
    print(blm2.check("this is a test"))
    print(blm2.check("blah"))
    print(blm2)
    print(blm2.bloom_array)

    blm2.add('yet another test')

    print("\n\ncheck intersection")
    blm3 = blm.intersection(blm2)
    print(blm3)
    print(blm3.check("this is a test"))
    print(blm3.check("yet another test"))

    print("\n\ncheck union")
    blm3 = blm.union(blm2)
    print(blm3)
    print(blm3.check("this is a test"))
    print(blm3.check("yet another test"))
    print(blm3.estimate_elements())

    print(blm.jaccard_index(blm2))

    print ('\n\nexport to hex')
    hex_out = blm.export_hex()
    print(hex_out)
    print('import hex')
    blm4 = BloomFilter()
    blm4.load_hex(hex_out)
    print(blm4)

    # on disk code check
    blmd = BloomFilterOnDisk()
    blmd.initialize('./dist/py_ondisk.blm', 10, 0.05)
    blmd.add('this is a test')
    print(blmd.current_false_positive_rate())
    print(blmd.estimate_elements())  # doesn't work with on disk
    print(blmd.export_size())
    print(blmd.check('this is a test'))
    print(blmd.check('this is a test... again'))
    print(blmd)

    # print(blmd.intersection(blm))  # doesn't work on disk... yet
    # print(blmd.union(blmd))  # doesn't work on disk... yet
    # print(blmd.jaccard_index(blm))  # doesn't work on disk... yet
