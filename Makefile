TESTDIR=tests
DISTDIR=dist
SRCDIR=src
CCFLAGS=-lm -Wall -Wpedantic -O3

all: clean bloom
	gcc $(DISTDIR)/bloom.o $(TESTDIR)/bloom_test.c $(CCFLAGS) -o ./$(DISTDIR)/blm
omp: bloom
	if [ -e ./$(DISTDIR)/blmmt ]; then rm ./dist/blmmt; fi;
	gcc $(DISTDIR)/bloom.o $(TESTDIR)/bloom_multi_thread.c $(CCFLAGS) -fopenmp -o ./$(DISTDIR)/blmmt
clean:
	rm -rf ./$(DISTDIR)/*
bloom:
	gcc -c $(SRCDIR)/bloom.c -o $(DISTDIR)/bloom.o $(CCFLAGS)
