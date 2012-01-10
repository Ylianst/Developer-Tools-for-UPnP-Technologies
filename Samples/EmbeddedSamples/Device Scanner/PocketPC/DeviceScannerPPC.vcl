<html>
<body>
<pre>
<h1>Build Log</h1>
<h3>
--------------------Configuration: DeviceScannerPPC - Win32 (WCE ARM) Release--------------------
</h3>
<h3>Command Lines</h3>
Creating temporary file "C:\DOCUME~1\YSAINTHI.AMR\LOCALS~1\Temp\RSP183.tmp" with contents
[
/nologo /W3 /D "ARM" /D "_ARM_" /D UNDER_CE=300 /D _WIN32_WCE=300 /D "WIN32_PLATFORM_PSPC=310" /D "UNICODE" /D "_UNICODE" /D "NDEBUG" /D "_AFXDLL" /D "MICROSTACK_NO_STDAFX" /Fo"ARMRel/" /Oxs /MC /c 
"C:\MicroStack\Device Scanner\PocketPC\UPnPControlPoint.c"
]
Creating command line "clarm.exe @C:\DOCUME~1\YSAINTHI.AMR\LOCALS~1\Temp\RSP183.tmp" 
Creating temporary file "C:\DOCUME~1\YSAINTHI.AMR\LOCALS~1\Temp\RSP184.tmp" with contents
[
/nologo /base:"0x00010000" /stack:0x10000,0x1000 /entry:"wWinMainCRTStartup" /incremental:no /pdb:"ARMRel/DeviceScannerPPC.pdb" /out:"ARMRel/DeviceScannerPPC.exe" /subsystem:windowsce,3.00 /align:"4096" /MACHINE:ARM 
".\ARMRel\DeviceScannerPPC.obj"
".\ARMRel\DeviceScannerPPCDlg.obj"
".\ARMRel\ILibHTTPClient.obj"
".\ARMRel\ILibMiniWebServer.obj"
".\ARMRel\ILibParsers.obj"
".\ARMRel\ILibSSDPClient.obj"
".\ARMRel\StdAfx.obj"
".\ARMRel\UPnPControlPoint.obj"
".\ARMRel\DeviceScannerPPC.res"
"..\..\..\Program Files\Windows CE Tools\wce300\Pocket PC 2002\lib\arm\winsock.lib"
"..\..\..\Program Files\Windows CE Tools\wce300\Pocket PC 2002\lib\arm\aygshell.lib"
]
Creating command line "link.exe @C:\DOCUME~1\YSAINTHI.AMR\LOCALS~1\Temp\RSP184.tmp"
<h3>Output Window</h3>
Compiling...
UPnPControlPoint.c
Linking...



<h3>Results</h3>
DeviceScannerPPC.exe - 0 error(s), 0 warning(s)
</pre>
</body>
</html>
