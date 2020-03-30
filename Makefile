TESTDIR=tests
DISTDIR=dist
SRCDIR=src
CCFLAGS=-lm -Wall -Wpedantic -Winline -O3

all: bloom
	gcc $(DISTDIR)/bloom.o $(TESTDIR)/bloom_test.c $(CCFLAGS) -o ./$(DISTDIR)/blm

omp: bloom
	if [ -e ./$(DISTDIR)/blmmt ]; then rm ./dist/blmmt; fi;
	gcc $(DISTDIR)/bloom.o $(TESTDIR)/bloom_multi_thread.c $(CCFLAGS) -fopenmp -o ./$(DISTDIR)/blmmt

clean:
	# library
	if [ -f "./$(DISTDIR)/bloom.o" ]; then rm -r ./$(DISTDIR)/bloom.o; fi
	# executables
	if [ -f "./$(DISTDIR)/blmmt" ]; then rm -r ./$(DISTDIR)/blmmt; fi
	if [ -f "./$(DISTDIR)/blm" ]; then rm -r ./$(DISTDIR)/blm; fi
	# test file
	if [ -f "./$(DISTDIR)/test_bloom.blm" ]; then rm -r ./$(DISTDIR)/test_bloom.blm; fi
bloom:
	gcc -c $(SRCDIR)/bloom.c -o $(DISTDIR)/bloom.o $(CCFLAGS)
