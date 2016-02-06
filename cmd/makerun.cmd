@echo off

cd ..\cmake

"c:\Program Files (x86)\CMake\bin\cmake.exe" -G "MinGW Makefiles" ..\src_c

"c:\MinGW\bin\mingw32-make.exe"

@rem ..\bin\SocketTestC.exe

pause