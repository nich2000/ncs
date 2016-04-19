@echo off

PATH=%PATH%;c:/CMake/bin/;c:/MinGW-w64/mingw64/bin/;

cd ../cmake
cmake.exe -G "MinGW Makefiles" ../src_c
mingw32-make.exe

rem pause
