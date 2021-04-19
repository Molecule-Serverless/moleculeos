## MoleculeOS

This is the globalOS of Molecule (or MoleculeOS).


### High-level Abstraction

MoleculeOS now provides:
- IPC (cross-device IPC, based on RDMA)
- Vector Cgroups


### Build

Enter the source dir:

	cd src

Build MoleculeOS:

	make

Build Demo Test:

	make tests

### Run Demos

Open three terminals.

Note: all the folloing commands happen on src/ dir.

In the first terminal, start moleculeOS:

	./moleculeos -i 0

Note: the -i here, means the PU-id of the globalOS

In the second terminal, run the server process:

	./test-fifo-lat-server

In the thrid terminal, run the client process:

	./test-fifo-lat-client -i 1 -s 64 -c 1000

Note:
- i: here means the global FIFO used by server, please check the value by seeing the logs in ./test-fifo-lat-server
- s: means the test size (e.g., 64 means 64Bytes)
- c: means the count to run test cases (e.g., 100 means run 100 times)


### Tests

MoleculeOS includes a set of unit test cases and regression test.

Please refer src/tests/README.md for more info.

Note: ensure you can pass regression test before you issue any PR/MR/updates.



#### (Example/ipc) User-client and User-server communicate

	cd src/
	make example-ipc
	./example-ipc-server&
	# You should see server's pid, e.g., 18855
	./example-ipc-client -i 18855 #Here, -i indicates the server's ID (pid

	#Now you should see the results of their communication

#### RDMA-IPC

In the server:
./write_lat -d mlx5_1 -i 1 -F -a

In the client:
./write_lat 192.168.120.1 -d mlx5_1 -i 1 -a -F

Note: the 192.168.120.1 is the server's IP (Server's NIC's IP)

#### Local-IPC

	cd src
	make local-ipc

	./local-ipc-server &
	# You should see the pid of local-ipc-server, e.g., 9583
	# Then, invoke the server using the pid
	./local-ipc-client -g 9583
