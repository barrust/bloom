TESTDIR=tests
DISTDIR=dist
SRCDIR=src
CCFLAGS=-lm -Wall -Wpedantic

all: clean bloom
	gcc $(SRCDIR)/bloom.o $(TESTDIR)/bloom_test.c $(CCFLAGS) -lcrypto -o ./$(DISTDIR)/blm
	gcc $(SRCDIR)/bloom.o $(TESTDIR)/bloom_export_import.c $(CCFLAGS) -o ./$(DISTDIR)/blmix
	gcc $(SRCDIR)/bloom.o $(TESTDIR)/bloom_on_disk.c $(CCFLAGS) -o ./$(DISTDIR)/blmd
omp: bloom
	if [ -e ./$(DISTDIR)/blmmt ]; then rm ./dist/blmmt; fi;
	gcc $(SRCDIR)/bloom.o $(TESTDIR)/bloom_multi_thread.c $(CCFLAGS) -fopenmp -o ./$(DISTDIR)/blmmt
clean:
	rm -rf ./$(DISTDIR)/*
	if [ -e ./$(SRCDIR)/bloom.o ]; then rm ./$(SRCDIR)/bloom.o; fi;
bloom:
	gcc -c $(SRCDIR)/bloom.c -o $(SRCDIR)/bloom.o $(CCFLAGS)
