''' BloomFilter, python implementation '''
from __future__ import print_function
import math, hashlib, struct, os

class BloomFilter(object):
    ''' '''
    def __init__(self):
        ''' setup the basic values needed '''
        self.bloom = None
        self.number_bits = 0  # number of bits
        self.est_elements = 0
        self.fpr = 0.0
        self.number_hashes = 0.0
        self.bloom_length = self.number_bits / 8
        self.hash_function = self.__default_hash
        self.els_added = 0
        self.__on_disk = False  # not on disk

    def __str__(self):
        ''' correctly handle python 3 vs python2 encoding if necessary '''
        return self.__unicode__()

    def __unicode__(self):
        ''' string / unicode representation of the bloom filter '''
        on_disk = "no" if self.__on_disk is False else "yes"
        stats = ('BloomFilter: \n'
                '\tbits: {0}\n'
                '\testimated elements: {1}\n'
                '\tnumber hashes: {2}\n'
                '\tmax false positive rate: {3:.6f}\n'
                '\tbloom length (8 bits): {4}\n'
                '\telements added: {5}\n'
                '\testimated elements added: {6}\n'
                '\tcurrent false positive rate: \n'
                '\texport size (bytes): {7}\n'
                '\tnumber bits set: {8}\n'
                '\tis on disk: {9}\n')
        return stats.format(self.number_bits, self.est_elements, self.number_hashes, self.fpr, self.bloom_length, self.els_added, self.estimate_elements(), self.export_size(), self.__number_bits_set(), on_disk)

    def init(self, estimated_elements, false_positive_rate, hash_function=None):
        ''' initialize the bloom filter '''
        self.__optimized_params(estimated_elements, false_positive_rate, 0, hash_function)

    def hashes(self, key, depth=None):
        ''' calculate the hashes for the passed in key '''
        tmp = depth if depth is not None else self.number_hashes
        return self.hash_function(key, tmp)

    def add(self, key):
        ''' add the key to the bloom filter '''
        hashes = self.hashes(key)
        self.add_alt(hashes)

    def add_alt(self, hashes):
        ''' add the element represented by hashes into the bloom filter '''
        for x in list(range(0, self.number_hashes)):
            k = int(hashes[x]) % self.number_bits
            idx = k // 8
            self.bloom[idx] = int(self.bloom[idx]) | int((1 << (k % 8)))
        self.els_added += 1

    def check(self, key):
        ''' check if the key is likely in the bloom filter '''
        hashes = self.hashes(key)
        return self.check_alt(hashes)

    def check_alt(self, hashes):
        ''' check if the element represented by hashes is in the bloom filter '''
        for x in list(range(0, self.number_hashes)):
            k = int(hashes[x]) % self.number_bits
            if (int(self.bloom[k // 8]) & int((1 << (k % 8)))) == 0:
                return False
        return True

    def intersection(self, second):
        ''' return a new Bloom Filter that contains the intersection of the two '''
        if self.__verify_bloom_similarity(second) is False:
            return None
        res = BloomFilter()
        res.init(self.est_elements, self.fpr, self.hash_function)

        for i in list(range(0, self.bloom_length)):
            res.bloom[i] = self.bloom[i] & second.bloom[i]
        res.els_added = res.estimate_elements()
        return res

    def union(self, second):
        ''' return a new Bloom Filter that contains the union of the two '''
        if self.__verify_bloom_similarity(second) is False:
            return None
        res = BloomFilter()
        res.init(self.est_elements, self.fpr, self.hash_function)

        for i in list(range(0, self.bloom_length)):
            res.bloom[i] = self.bloom[i] | second.bloom[i]
        res.els_added = res.estimate_elements()
        return res

    def jaccard_index(self, second):
        ''' calculate the jaccard similarity score '''
        if self.__verify_bloom_similarity(second) is False:
            return None
        count_union = 0
        count_intersection = 0
        for i in list(range(0, self.bloom_length)):
            count_union += self.__set_bits(self.bloom[i] | second.bloom[i])
            count_intersection += self.__set_bits(self.bloom[i] & second.bloom[i])
        if count_union == 0:
            return 1.0
        return float(count_intersection) / float(count_union)

    def export(self, filename):
        ''' export the bloom filter to disk '''
        with open(filename, 'wb') as fp:
            for x in list(range(0, self.bloom_length)):
                fp.write(struct.pack('B', int(self.bloom[x])))
            fp.flush()
            fp.write(struct.pack('QQf', self.est_elements, self.els_added, self.fpr))
            fp.flush()


    def load(self, filename, hash_function=None):
        ''' load the bloom filter from file '''
        # read in the needed information, and then call __optimized_params
        # to set everything correctly
        with open(filename, 'rb') as fp:
            offset = struct.calcsize('QQf')
            fp.seek(offset * -1, os.SEEK_END)
            mybytes = struct.unpack('QQf', fp.read(offset))
            self.est_elements = mybytes[0]
            self.els_added = mybytes[1]
            self.fpr = mybytes[2]

            self.__optimized_params(self.est_elements, self.fpr, self.els_added, hash_function)

            fp.seek(0, os.SEEK_SET)
            bytesize = struct.calcsize('B')
            # now read in the bit array!
            for i in list(range(0, self.bloom_length)):
                val = struct.unpack('B', fp.read(bytesize))[0]
                self.bloom[i] = val

    def export_hex(self):
        pass

    def load_hex(self, string):
        pass

    def export_size(self):
        ''' calculate the size of the bloom on disk '''
        return (self.bloom_length * struct.calcsize('B')) + struct.calcsize('QQf')

    def estimate_elements(self):
        ''' estimate the number of elements added '''
        setbits = self.__number_bits_set()
        log_n = math.log(1 - (float(setbits) / float(self.number_bits)))
        return int(-1 * (float(self.number_bits) / float(self.number_hashes)) * log_n)

    def __optimized_params(self, estimated_elements, false_positive_rate, elements_added, hash_function):
        ''' set the parameters to the optimal sizes '''
        self.hash_function = hash_function if hash_function is not None else self.__default_hash
        self.est_elements = estimated_elements
        fpr = struct.pack('f', float(false_positive_rate))
        self.fpr = struct.unpack('f', fpr)[0] # to mimic the c version!
        # self.fpr = false_positive_rate
        self.els_added = elements_added
        # optimal caluclations
        n = self.est_elements
        p = float(self.fpr)
        m = math.ceil((-n * math.log(p)) / 0.4804530139182) # LOG_TWO_SQUARED
        k = round(math.log(2.0) * m / n)
        self.number_hashes = int(k)
        self.number_bits = int(m)
        self.bloom_length = int(math.ceil(m / (8 * 1.0)))
        self.bloom = [0] * self.bloom_length

    def __verify_bloom_similarity(self, second):
        ''' can the blooms be used in intersection, union, or jaccard index '''
        if self.number_hashes != second.number_hashes or self.number_bits != second.number_bits or self.hashes("test") != second.hashes("test"):
            return False
        return True

    @staticmethod
    def __set_bits(i):
        ''' count number of bits set '''
        return bin(i).count("1")

    def __number_bits_set(self):
        ''' calculate the total number of set bits in the bloom '''
        setbits = 0
        for i in list(range(0, self.bloom_length)):
            setbits += self.__set_bits(self.bloom[i])
        return setbits

    def __default_hash(self, key, depth):
        ''' the default fnv-1a hashing routine '''
        res = list()
        tmp = key
        for i in list(range(0, depth)):
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
        for s in key:
            hval = hval ^ ord(s)
            hval = (hval * fnv_64_prime) % uint64_max
        return hval



if __name__ == '__main__':
    blm = BloomFilter()
    blm.init(10, 0.05)
    blm.add("this is a test")
    print(blm.check("this is a test"))
    print(blm.check("blah"))
    print(blm)
    print(blm.bloom)
    blm.export('./dist/py_bloom.blm')

    print('\n\ncheck imported BloomFilter!')

    blm2 = BloomFilter()
    blm2.load('./dist/py_bloom.blm')
    print(blm2.check("this is a test"))
    print(blm2.check("blah"))
    print(blm2)
    print(blm2.bloom)

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
