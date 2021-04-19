all: test_hashmap

test_hashmap: hashmap.c test_hashmap.c
	gcc -o $@ $^ -I.

test: test_hashmap
	BENCH=1 ./test_hashmap


clean:
	rm *.o test_hashmap -f


