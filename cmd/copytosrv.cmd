cd /d "c:\PuTTY\"

pscp.exe -P 22 -r -scp "e:\DELPHI Projects\_TestArea\ProtocolTest\src_c" nich@10.0.10.12:/home/nich/Protocol/

@rem pscp.exe -P 22 -scp "e:\DELPHI Projects\_TestArea\ProtocolTest\cmd\makerun.sh" nich@10.0.10.45:/home/nich/Protocol/

@rem pscp.exe -P 22 -scp "e:\DELPHI Projects\_TestArea\ProtocolTest\cmd\run.sh" nich@10.0.10.45:/home/nich/Protocol/

pause