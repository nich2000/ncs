@echo off

cd ../bin

@rem del log.txt

SocketTestC -c -p 5700 -h 127.0.0.1
@rem SocketTestC -c -p 5700 -h 176.117.126.134

pause