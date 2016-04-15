@echo off

cd .\cmake

"c:\CMake\bin\cmake.exe" -G "MinGW Makefiles" ..\src_c
rem "c:\CMake\bin\cmake.exe" -D "CMAKE_MAKE_PROGRAM:PATH=c:/Qt/Tools/mingw492_32/bin/mingw32-make.exe" ..\src_c

"c:\Qt\Tools\mingw492_32\bin\mingw32-make.exe"
