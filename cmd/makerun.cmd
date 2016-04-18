@echo off

PATH=%PATH%;c:/MinGW/bin/;

cd ../cmake

"c:/CMake/bin/cmake.exe" -G "MinGW Makefiles" ../src_c

pause

"c:/MinGW/bin/mingw32-make.exe"

pause
