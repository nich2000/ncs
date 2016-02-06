#!/bin/sh

clear

mkdir cmake

cd ./cmake
cmake ../src_c
make

cd ../bin
./SocketTestC
