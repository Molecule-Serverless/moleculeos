## MoleculeOS

This is the globalOS of Molecule (or MoleculeOS).


### High-level Abstraction

MoleculeOS now provides:
- IPC (cross-device IPC, based on RDMA)
- cfork (@lqy)


### Build

	cd src
	make

### Demos


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
