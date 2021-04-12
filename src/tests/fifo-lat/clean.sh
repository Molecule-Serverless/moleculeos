#!/bin/bash


sudo kill -9 $(pgrep moleculeos-dist)
sudo kill -9 $(pgrep test-fifo-lat-server)

