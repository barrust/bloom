all:
	gcc bloom.c bloom_test.c -lm -lcrypto -o ./dist/blm
clean:
	if [ -e ./dist/blm ]; then rm -L. ./dist/blm; fi;
