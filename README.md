## MoleculeOS

This is the globalOS of Molecule (or MoleculeOS).


### High-level Abstraction

MoleculeOS now provides:
- IPC (cross-device IPC, based on RDMA)
- cfork (@lqy)


### Build

	cd src
	make

### Run

In the server:
./write_lat -d mlx5_1 -i 1 -F -a

In the client:
./write_lat 192.168.120.1 -d mlx5_1 -i 1 -a -F

Note: the 192.168.120.1 is the server's IP (Server's NIC's IP)

