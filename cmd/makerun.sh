#!/bin/sh

PATH=$PATH:/home/pi/Downloads/cmake-3.2.1.build/bin

clear

mkdir cmake
mkdir bin

cd ./cmake
cmake ../src_c
make
