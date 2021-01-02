CC=gcc
COMPFLAGS=-lm -Wall -Wpedantic -Winline -Wextra -Wno-long-long
DISTDIR=dist
SRCDIR=src
TESTDIR=tests
UNKNOWN_PRAGMAS=-Wno-unknown-pragmas

all: bloom
	$(CC) ./$(DISTDIR)/bloom.o ./$(TESTDIR)/bloom_test.c $(CCFLAGS) $(COMPFLAGS) $(UNKNOWN_PRAGMAS) -o ./$(DISTDIR)/blm

# add openmp and keep unknown pragmas
omp: COMPFLAGS += -fopenmp
omp: UNKNOWN_PRAGMAS=
omp: all
	$(CC) ./$(DISTDIR)/bloom.o ./$(TESTDIR)/bloom_multi_thread.c $(CCFLAGS) $(COMPFLAGS) $(UNKNOWN_PRAGMAS) -o ./$(DISTDIR)/blmmt

debug: COMPFLAGS += -g
debug: all

release: COMPFLAGS += -O3
release: all

sanitize: COMPFLAGS += -fsanitize=undefined
sanitize: test

test: COMPFLAGS += -coverage
test: bloom
	$(CC) ./$(DISTDIR)/bloom.o ./$(TESTDIR)/testsuite.c $(CCFLAGS) $(COMPFLAGS) $(UNKNOWN_PRAGMAS) -o ./$(DISTDIR)/test -g -lcrypto

runtests:
	@ if [ -f "./$(DISTDIR)/test" ]; then ./$(DISTDIR)/test; fi

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
	$(CC) -c ./$(SRCDIR)/bloom.c -o ./$(DISTDIR)/bloom.o $(CCFLAGS) $(COMPFLAGS) $(UNKNOWN_PRAGMAS)
