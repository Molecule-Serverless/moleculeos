#!/bin/bash


echo FIFO_LAT_test begin

./moleculeos -i 0 &
sleep 2


## Test
./test-fifo-lat-server &
sleep 2

# Here, we assum the global-fifo used by server is #1
./test-fifo-lat-client -i 1 -p 65259 -s 16 -c 300
./test-fifo-lat-client -i 1 -p 65259 -s 32 -c 300
./test-fifo-lat-client -i 1 -p 65259 -s 64 -c 300
./test-fifo-lat-client -i 1 -p 65259 -s 128 -c 300
./test-fifo-lat-client -i 1 -p 65259 -s 256 -c 300
./test-fifo-lat-client -i 1 -p 65259 -s 512 -c 300
./test-fifo-lat-client -i 1 -p 65259 -s 1024 -c 300
./test-fifo-lat-client -i 1 -p 65259 -s 2048 -c 300

echo FIFO_LAT_test finished

sudo kill -9 $(pgrep moleculeos) -f
sudo kill -9 $(pgrep test-fifo-lat-server) -f
