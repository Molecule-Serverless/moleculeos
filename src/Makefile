RDMA_LIB_SOURCES_files= get_clock.c multicast_resources.c perftest_communication.c perftest_counters.c perftest_parameters.c perftest_resources.c raw_ethernet_resources.c
LOCAL_IPC_LIB_SOURCES_files= common/arguments.c \
common/benchmarks.c \
common/parent.c \
common/process.c \
common/signals.c \
common/sockets.c \
common/utility.c 

SHM_GSYSCALL_CHANNEL_SOURCES = shm-gsyscall-channel/shm_gsyscall_server.cc
SHM_GSYSCALL_CHANNEL_OBJS = shm_gsyscall_server.o

#DSM_SOURCES_files= inter-device/molecule_dsm.c\
#		   inter-device/dsm_handlers.c

DSM_SOURCES_files= inter-device/uct_molecule_dsm.c\
		   inter-device/dsm_handlers.c

RDMA_LIB_SOURCES = $(addprefix rdma-lib/, $(RDMA_LIB_SOURCES_files))
LOCAL_IPC_LIB_SOURCES = $(addprefix local-ipc/, $(LOCAL_IPC_LIB_SOURCES_files))
SYSCALL_RUNTIME_LIB_SOURCES = global_syscall_interfaces.c global_syscall_runtime.c

UCX_LIB?=../../ucx/build/lib/
UCX_INCLUDE?=../../ucx/build/include/

USER_LIB_SOURCES = ipc.c $(LOCAL_IPC_LIB_SOURCES) $(SYSCALL_RUNTIME_LIB_SOURCES)
USER_LIB_OBJS = $(patsubst %.c,%.o, $(notdir $(USER_LIB_SOURCES)))

All: moleculeos

## A Local Development Version of MoleculeOS
moleculeos: moleculeos.c global_syscall.c ipc.c util/hashtable.c $(LOCAL_IPC_LIB_SOURCES) $(SHM_GSYSCALL_CHANNEL_OBJS)
	gcc -o $@ $^ -Iinclude -Ilocal-ipc/ -lm -lpthread -lstdc++ -lrt

## A Dist Version of MoleculeOS, it's based on DSM to communicate with other PUs
moleculeos-dist: moleculeos.c global_syscall.c ipc.c util/hashtable.c $(LOCAL_IPC_LIB_SOURCES) $(DSM_SOURCES_files)
	#gcc -o $@ $^ -Iinclude -Ilocal-ipc/ -I$(UCX_INCLUDE) -L$(UCX_LIB) -lucp -lucs -luct -lm -lpthread -DSMARTC
	gcc -o $@ $^ -Iinclude -Ilocal-ipc/ -I$(UCX_INCLUDE) -L$(UCX_LIB) -lucp -lucs -luct -lm -lpthread -DSMARTC -DMOLECULE_CLEAN 
	@echo build $@ successfully, please set LD_LIBRARY_PATH, e.g., export LD_LIBRARY_PATH=$(UCX_LIB) before you run

$(SHM_GSYSCALL_CHANNEL_OBJS):  $(SHM_GSYSCALL_CHANNEL_SOURCES)
	g++ -std=c++11 -O3 -DUSE_SHM=1 -c $^ -o $@ -Iinclude -lrt

molecule-rdma: molecule-rdma.c $(RDMA_LIB_SOURCES)
	gcc -o $@ $^ -Irdma-lib/ -libumad -lm -lm  -lmlx5 -lrdmacm -libverbs -lpthread  -lresolv -D_GNU_SOURCE

local-ipc: local-ipc-client local-ipc-server
	@echo build local demo success

local-ipc-client: local-ipc/fifo/client.c $(LOCAL_IPC_LIB_SOURCES)
	gcc -o $@ $^ -Ilocal-ipc/ -lm -lpthread

local-ipc-server: local-ipc/fifo/server.c $(LOCAL_IPC_LIB_SOURCES)
	gcc -o $@ $^ -Ilocal-ipc/ -lm -lpthread

write_lat: write_lat.c $(LIB_SOURCES)
	gcc -o write_lat *.c -libumad -lm -lm  -lmlx5 -lrdmacm -libverbs -lpthread  -lresolv -D_GNU_SOURCE

example-ipc: example-ipc-client example-ipc-server
	@echo build example/ipc success

example-neighbor-ipc: example-neighboripc-client example-neighboripc-server
	@echo build example/neightbor-ipc success
	@echo To run the demo:
	@echo Start globalOS: ./moleculeos -i 0
	@echo ./example-neighboripc-server
	@echo Read the Server global fifo value, e.g.,1
	@echo ./example-neighboripc-client -i server_global_fifo_value

tests: test-fifo-lat test-gSpawn test-fifo-lat-uuid
	@echo build tests successfully
	@echo Please follow the scripts/README in each test dir to run them

regression: tests
	./tests/regression_test.sh

test-gSpawn: test-gSpawn-lat-client test-gSpawn-lat-child
test-fifo-lat: test-fifo-lat-client test-fifo-lat-server
test-fifo-lat-uuid: test-fifo-lat-client-uuid test-fifo-lat-server-uuid

test-gSpawn-lat-client: tests/gSpawn-lat/client.c ipc.c $(LOCAL_IPC_LIB_SOURCES) $(SYSCALL_RUNTIME_LIB_SOURCES)
	gcc -o $@ $^ -Ilocal-ipc/ -Iinclude/ -lm -lpthread -DMOLECULE_CLEAN

test-gSpawn-lat-child: tests/gSpawn-lat/child.c ipc.c $(LOCAL_IPC_LIB_SOURCES) $(SYSCALL_RUNTIME_LIB_SOURCES)
	gcc -o $@ $^ -Ilocal-ipc/ -Iinclude/ -lm -lpthread -DMOLECULE_CLEAN

test-fifo-lat-client: tests/fifo-lat/client.c ipc.c $(LOCAL_IPC_LIB_SOURCES) $(SYSCALL_RUNTIME_LIB_SOURCES)
	gcc -o $@ $^ -Ilocal-ipc/ -Iinclude/ -lm -lpthread -DMOLECULE_CLEAN

test-fifo-lat-server: tests/fifo-lat/server.c ipc.c $(LOCAL_IPC_LIB_SOURCES) $(SYSCALL_RUNTIME_LIB_SOURCES)
	gcc -o $@ $^ -Ilocal-ipc/ -Iinclude/ -lm -lpthread -DMOLECULE_CLEAN

test-fifo-lat-client-uuid: tests/fifo-lat-uuid/client.c ipc.c $(LOCAL_IPC_LIB_SOURCES) $(SYSCALL_RUNTIME_LIB_SOURCES)
	gcc -o $@ $^ -Ilocal-ipc/ -Iinclude/ -lm -lpthread -DMOLECULE_CLEAN

test-fifo-lat-server-uuid: tests/fifo-lat-uuid/server.c ipc.c $(LOCAL_IPC_LIB_SOURCES) $(SYSCALL_RUNTIME_LIB_SOURCES)
	gcc -o $@ $^ -Ilocal-ipc/ -Iinclude/ -lm -lpthread -DMOLECULE_CLEAN

userlib: $(USER_LIB_SOURCES)
	@rm ./moleculeOS-userlib/ -rf
	@mkdir ./moleculeOS-userlib
	@cp ipc.c local-ipc $(SYSCALL_RUNTIME_LIB_SOURCES) ./moleculeOS-userlib/ -r
	@mkdir -p ./moleculeOS-userlib/include
	@cp include/global_syscall_interfaces.h ./moleculeOS-userlib/include/
	@cp include/global_syscall_protocol.h ./moleculeOS-userlib/include/
	@cp include/global_syscall_runtime.h ./moleculeOS-userlib/include/
	@cp include/molecule-ipc.h ./moleculeOS-userlib/include/
	@cp include/chos ./moleculeOS-userlib/include/ -r
	@gcc -c -fPIC $^ -Ilocal-ipc/ -Iinclude/ -DMOLECULE_CLEAN
	@ar rcs libmoleculeos.a $(USER_LIB_OBJS)
	@echo Build Userlib sucessfully, static lib is libmoleculeos.a
	@echo You can also use source code in moleculeOS-userlib, and include files: -ImoleculeOS-userlib/include and -ImoleculeOS-userlib/local-ipc


example-ipc-client: examples/ipc/client.c ipc.c $(LOCAL_IPC_LIB_SOURCES)
	gcc -o $@ $^ -Ilocal-ipc/ -Iinclude/ -lm -lpthread

example-ipc-server: examples/ipc/server.c ipc.c $(LOCAL_IPC_LIB_SOURCES)
	gcc -o $@ $^ -Ilocal-ipc/ -Iinclude/ -lm -lpthread

example-neighboripc-client: examples/neighbor-ipc/client.c ipc.c $(LOCAL_IPC_LIB_SOURCES) $(SYSCALL_RUNTIME_LIB_SOURCES)
	gcc -o $@ $^ -Ilocal-ipc/ -Iinclude/ -lm -lpthread

example-neighboripc-server: examples/neighbor-ipc/server.c ipc.c $(LOCAL_IPC_LIB_SOURCES) $(SYSCALL_RUNTIME_LIB_SOURCES)
	gcc -o $@ $^ -Ilocal-ipc/ -Iinclude/ -lm -lpthread

clean:
	rm *.o write_lat -f user-client user-server example-ipc-client example-ipc-server example-neighboripc-client example-neighboripc-server global_syscall globalos molecule-rdma moleculeos test-fifo-lat-client test-fifo-lat-server test-gSpawn-lat-child test-gSpawn-lat-client test-fifo-lat-client-uuid test-fifo-lat-server-uuid
