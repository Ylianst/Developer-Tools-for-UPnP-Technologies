<html>
<body>
<pre>
<h1>Build Log</h1>
<h3>
--------------------Configuration: MediaBrowserPPC - Win32 (WCE ARM) Debug--------------------
</h3>
<h3>Command Lines</h3>
Creating temporary file "C:\DOCUME~1\nkidd\LOCALS~1\Temp\RSP317.tmp" with contents
[
/nologo /W3 /Zi /Od /I "..\\" /I "..\MediaBrowserPPC" /D "DEBUG" /D "ARM" /D "_ARM_" /D "MICROSTACK_NO_STDAFX" /D UNDER_CE=300 /D _WIN32_WCE=300 /D "WIN32_PLATFORM_PSPC=310" /D "UNICODE" /D "_UNICODE" /D "_AFXDLL" /D "MMSCP_LEAN_AND_MEAN" /FR"ARMDbg/" /Fo"ARMDbg/" /Fd"ARMDbg/" /MC /c 
"C:\cygwin\home\Administrator\Micro AV Media Browser\MediaBrowserPPC\MediaBrowserPPCDlg.cpp"
]
Creating command line "clarm.exe @C:\DOCUME~1\nkidd\LOCALS~1\Temp\RSP317.tmp" 
Creating temporary file "C:\DOCUME~1\nkidd\LOCALS~1\Temp\RSP318.tmp" with contents
[
/nologo /base:"0x00010000" /stack:0x10000,0x1000 /entry:"wWinMainCRTStartup" /incremental:yes /pdb:"ARMDbg/Media Browser.pdb" /debug /out:"ARMDbg/Media Browser.exe" /subsystem:windowsce,3.00 /align:"4096" /MACHINE:ARM 
".\ARMDbg\ILibHTTPClient.obj"
".\ARMDbg\ILibMiniWebServer.obj"
".\ARMDbg\ILibParsers.obj"
".\ARMDbg\ILibSSDPClient.obj"
".\ARMDbg\MediaBrowserPPC.obj"
".\ARMDbg\MediaBrowserPPCDlg.obj"
".\ARMDbg\MmsCp.obj"
".\ARMDbg\MSCPControlPoint.obj"
".\ARMDbg\MyString.obj"
".\ARMDbg\StdAfx.obj"
".\ARMDbg\MediaBrowserPPC.res"
".\winsock_arm.lib"
]
Creating command line "link.exe @C:\DOCUME~1\nkidd\LOCALS~1\Temp\RSP318.tmp"
<h3>Output Window</h3>
Compiling...
MediaBrowserPPCDlg.cpp
Linking...
Creating temporary file "C:\DOCUME~1\nkidd\LOCALS~1\Temp\RSP31B.tmp" with contents
[
/nologo /o"ARMDbg/MediaBrowserPPC.bsc" 
".\ARMDbg\StdAfx.sbr"
".\ARMDbg\ILibHTTPClient.sbr"
".\ARMDbg\ILibMiniWebServer.sbr"
".\ARMDbg\ILibParsers.sbr"
".\ARMDbg\ILibSSDPClient.sbr"
".\ARMDbg\MediaBrowserPPC.sbr"
".\ARMDbg\MediaBrowserPPCDlg.sbr"
".\ARMDbg\MmsCp.sbr"
".\ARMDbg\MSCPControlPoint.sbr"
".\ARMDbg\MyString.sbr"]
Creating command line "bscmake.exe @C:\DOCUME~1\nkidd\LOCALS~1\Temp\RSP31B.tmp"
Creating browse info file...
<h3>Output Window</h3>



<h3>Results</h3>
Media Browser.exe - 0 error(s), 0 warning(s)
</pre>
</body>
</html>
