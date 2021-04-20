all: test

test_hashmap: hashmap.c test_hashmap.c
	gcc -o $@ $^ -I.

test_hashtable: hashtable.c test_hashtable.c
	gcc -o $@ $^ -I.

test: test_hashmap test_hashtable
	BENCH=1 ./test_hashmap
	BENCH=1 ./test_hashtable


clean:
	rm *.o test_hashmap test_hashtable -f


