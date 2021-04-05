RDMA_LIB_SOURCES_files= get_clock.c multicast_resources.c perftest_communication.c perftest_counters.c perftest_parameters.c perftest_resources.c raw_ethernet_resources.c
LOCAL_IPC_LIB_SOURCES_files= common/arguments.c \
common/benchmarks.c \
common/parent.c \
common/process.c \
common/signals.c \
common/sockets.c \
common/utility.c 


RDMA_LIB_SOURCES = $(addprefix rdma-lib/, $(RDMA_LIB_SOURCES_files))
LOCAL_IPC_LIB_SOURCES = $(addprefix local-ipc/, $(LOCAL_IPC_LIB_SOURCES_files))

All: molecule-ipc

molecule-ipc: molecule-ipc.c $(RDMA_LIB_SOURCES)
	gcc -o $@ $^ -Irdma-lib/ -libumad -lm -lm  -lmlx5 -lrdmacm -libverbs -lpthread  -lresolv -D_GNU_SOURCE

local-ipc: local-ipc-client local-ipc-server
	@echo build local demo success

local-ipc-client: local-ipc/fifo/client.c $(LOCAL_IPC_LIB_SOURCES)
	gcc -o $@ $^ -Ilocal-ipc/ -lm -lpthread

local-ipc-server: local-ipc/fifo/server.c $(LOCAL_IPC_LIB_SOURCES)
	gcc -o $@ $^ -Ilocal-ipc/ -lm -lpthread

write_lat: write_lat.c $(LIB_SOURCES)
	gcc -o write_lat *.c -libumad -lm -lm  -lmlx5 -lrdmacm -libverbs -lpthread  -lresolv -D_GNU_SOURCE


clean:
	rm *.o write_lat -f
