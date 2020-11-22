CC=gcc
COMPFLAGS=-lm -Wall -Wpedantic -Winline -Wextra
DISTDIR=dist
SRCDIR=src
TESTDIR=tests

all: bloom
	$(CC) $(DISTDIR)/bloom.o $(TESTDIR)/bloom_test.c $(CCFLAGS) $(COMPFLAGS) -o ./$(DISTDIR)/blm

omp: bloom
	$(CC) $(DISTDIR)/bloom.o $(TESTDIR)/bloom_multi_thread.c $(CCFLAGS) $(COMPFLAGS) -fopenmp -o ./$(DISTDIR)/blmmt

debug: COMPFLAGS += -g
debug: all

release: COMPFLAGS += -O3
release: all

sanitize: COMPFLAGS += -fsanitize=undefined
sanitize: test

test: COMPFLAGS += -coverage
test: bloom
	$(CC) $(DISTDIR)/bloom.o $(TESTDIR)/testsuite.c $(CCFLAGS) $(COMPFLAGS) -o ./$(DISTDIR)/test -g -lcrypto

clean:
	# library
	if [ -f "./$(DISTDIR)/bloom.o" ]; then rm -r ./$(DISTDIR)/bloom.o; fi
	# executables
	if [ -f "./$(DISTDIR)/blmmt" ]; then rm -r ./$(DISTDIR)/blmmt; fi
	if [ -f "./$(DISTDIR)/blm" ]; then rm -r ./$(DISTDIR)/blm; fi
	# test file
	if [ -f "./$(DISTDIR)/test_bloom.blm" ]; then rm -r ./$(DISTDIR)/test_bloom.blm; fi
	# remove coverage items
	if [ -f "./$(DISTDIR)/test" ]; then rm -rf ./$(DISTDIR)/*.gcno; fi
	if [ -f "./$(DISTDIR)/test" ]; then rm -rf ./$(DISTDIR)/*.gcda; fi
	if [ -f "./$(DISTDIR)/test" ]; then rm -r ./$(DISTDIR)/test; fi
	rm -f ./*.gcno
	rm -f ./*.gcda
	rm -f ./*.gcov

bloom:
	$(CC) -c $(SRCDIR)/bloom.c -o $(DISTDIR)/bloom.o $(CCFLAGS) $(COMPFLAGS)
