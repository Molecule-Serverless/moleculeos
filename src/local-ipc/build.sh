#!/bin/bash

## Build fifo-client
gcc -o fifo-client fifo/client.c common/arguments.c common/benchmarks.c common/parent.c common/process.c common/signals.c common/sockets.c common/utility.c -I. -lpthread -lm

## Build fifo-server
gcc -o fifo-server fifo/server.c common/arguments.c common/benchmarks.c common/parent.c common/process.c common/signals.c common/sockets.c common/utility.c -I. -lpthread -lm
