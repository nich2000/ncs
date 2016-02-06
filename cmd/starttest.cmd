@echo off

cd ../bin

@rem del log.txt

start /b SocketTestC -s -p 5600

start /b SocketTestC -c -p 5600 -h 127.0.0.1

@rem start /b SocketTest -c -p 5600 -h 127.0.0.1

pause