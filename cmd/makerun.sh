#!/bin/sh

clear

mkdir cmake
mkdir bin

cd ./cmake
cmake ../src_c
make

#cd ../bin
#./SocketTestC
