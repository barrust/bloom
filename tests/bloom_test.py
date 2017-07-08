''' Test the functionality of the HeavyHitters class '''
from __future__ import (print_function)
import sys
sys.dont_write_bytecode = True
sys.path.append("./python")
from bloom import (BloomFilter, BloomFilterOnDisk)


def test():
    ''' basic testing functions '''
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
    print('\n\ntest using `in`')
    print("this is a test" in blm3)
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
    print('\n\nbloom filter on disk')
    blmd = BloomFilterOnDisk()
    blmd.initialize('./dist/py_ondisk.blm', 10, 0.05)
    blmd.add("this is a test")
    print(blmd.check('this is a test'))
    print('Check use of in keyword ("this is a test" in blmd): ',
          'this is a test' in blmd)
    print(blmd.check('yet another test'))
    # blmd.union(blm4)
    # blmd.intersection(blm)
    # print(blmd.jaccard_index(blm2))
    print(blmd)
    # print ('\n\nexport to hex')
    # hex_out = blmd.export_hex()
    # print(hex_out)
    blmd.close()

if __name__ == '__main__':
    test()
