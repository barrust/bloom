TESTDIR=tests
SRCDIR=src
CCFLAGS=-lm -Wall

all: clean bloom
	gcc $(SRCDIR)/bloom.o $(TESTDIR)/bloom_test.c $(CCFLAGS) -lcrypto -o ./dist/blm
	gcc $(SRCDIR)/bloom.o $(TESTDIR)/bloom_export_import.c $(CCFLAGS) -o ./dist/blmix
	gcc $(SRCDIR)/bloom.o $(TESTDIR)/bloom_on_disk.c $(CCFLAGS) -o ./dist/blmd
omp:
	if [ -e ./dist/blmmt ]; then rm ./dist/blmmt; fi;
	gcc bloom.o $(TESTDIR)/bloom_multi_thread.c $(CCFLAGS) -fopenmp -o ./dist/blmmt
clean:
	rm -rf ./dist/*
	if [ -e ./$(SRCDIR)/bloom.o ]; then rm ./$(SRCDIR)/bloom.o; fi;
bloom:
	gcc -c $(SRCDIR)/bloom.c -o $(SRCDIR)/bloom.o
