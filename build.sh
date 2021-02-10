#!/bin/bash

# removing previously built binary
rm producer_consumer

# cleaning old build
rm -rf build/

# creating build folder
mkdir build

cd build/

# preparation of Makefile
cmake ..

# compile & build
make

# go back to parent dir
cp producer_consumer ../
cd ..

# starting binary
# ./producer_consumer
