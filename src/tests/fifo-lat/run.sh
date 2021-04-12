#!/bin/bash


./moleculeos-dist -i 0 &
sleep 2

./moleculeos-dist -i 1 &
sleep 2

./test-fifo-lat-server &
sleep 2

# Here, we assum the global-fifo used by server is #1
./test-fifo-lat-client -i 1 -p 65260 -s 16 -c 100
./test-fifo-lat-client -i 1 -p 65260 -s 32 -c 100
./test-fifo-lat-client -i 1 -p 65260 -s 64 -c 100
./test-fifo-lat-client -i 1 -p 65260 -s 128 -c 100
./test-fifo-lat-client -i 1 -p 65260 -s 256 -c 100
./test-fifo-lat-client -i 1 -p 65260 -s 512 -c 100
./test-fifo-lat-client -i 1 -p 65260 -s 1024 -c 100
./test-fifo-lat-client -i 1 -p 65260 -s 2048 -c 100
