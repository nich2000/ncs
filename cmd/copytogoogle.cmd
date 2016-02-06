cd ../bin
copy liblog.dll c:\Google\ProtocolTest\bin\
copy libprotocol.dll c:\Google\ProtocolTest\bin\
copy libsocket.dll c:\Google\ProtocolTest\bin\
copy libwinpthread-1.dll c:\Google\ProtocolTest\bin\
copy SocketTestC.exe c:\Google\ProtocolTest\bin\

cd ../
Xcopy /Y cmd c:\Google\ProtocolTest\cmd
Xcopy /Y doc c:\Google\ProtocolTest\doc
Xcopy /Y examples c:\Google\ProtocolTest\examples
@rem Какого-то х.. не могу разобраться с /EXCLUDE:donotcopy.txt
@rem Xcopy /Y /EXCLUDE:donotcopy.txt src_c c:\Google\ProtocolTest\src_c 
Xcopy /Y src_c c:\Google\ProtocolTest\src_c 
Xcopy /Y src_delphi c:\Google\ProtocolTest\src_delphi 

pause