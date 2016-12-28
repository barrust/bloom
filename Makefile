TEST_DIR=tests
CCFLAGS=-lm -Wall

all: clean bloom
	gcc bloom.o $(TEST_DIR)/bloom_test.c $(CCFLAGS) -lcrypto -o ./dist/blm
	gcc bloom.o $(TEST_DIR)/bloom_export_import.c $(CCFLAGS) -o ./dist/blmix
	gcc bloom.o $(TEST_DIR)/bloom_on_disk.c $(CCFLAGS) -o ./dist/blmd
omp:
	if [ -e ./dist/blmmt ]; then rm ./dist/blmmt; fi;
	gcc bloom.o $(TEST_DIR)/bloom_multi_thread.c $(CCFLAGS) -fopenmp -o ./dist/blmmt
clean:
	if [ -e ./dist/blm ]; then rm ./dist/blm; fi;
	if [ -e ./dist/blmix ]; then rm ./dist/blmix; fi;
	if [ -e ./dist/blmd ]; then rm ./dist/blmd; fi;
	if [ -e ./dist/test.blm ]; then rm ./dist/test.blm; fi;
	if [ -e ./dist/test2.blm ]; then rm ./dist/test2.blm; fi;
	if [ -e ./dist/blmmt ]; then rm ./dist/blmmt; fi;
	if [ -e bloom.o ]; then rm bloom.o; fi;
bloom:
	gcc -c bloom.c -o bloom.o
