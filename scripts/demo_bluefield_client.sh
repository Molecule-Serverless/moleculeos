#!/bin/bash

# Note: 192.168.120.1 is the server addr (smartNIC)
./src/molecule-ipc 192.168.120.1 -d mlx5_1 -i 1 -a -F
