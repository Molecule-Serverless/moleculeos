LIB_SOURCES= get_clock.c multicast_resources.c perftest_communication.c perftest_counters.c perftest_parameters.c perftest_resources.c raw_ethernet_resources.c

all: write_lat.c $(LIB_SOURCES)
	gcc -o write_lat *.c -libumad -lm -lm  -lmlx5 -lrdmacm -libverbs -lpthread  -lresolv -D_GNU_SOURCE

molecule-ipc: molecule-ipc.c $(LIB_SOURCES)
	gcc -o $@ $^ -libumad -lm -lm  -lmlx5 -lrdmacm -libverbs -lpthread  -lresolv -D_GNU_SOURCE

clean:
	rm *.o write_lat -f
