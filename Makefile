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
	if [ -d "./$(DISTDIR)/" ]; then rm -rf ./$(DISTDIR)/*; fi
bloom:
	gcc -c $(SRCDIR)/bloom.c -o $(DISTDIR)/bloom.o $(CCFLAGS)
