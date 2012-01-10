<html>
<body>
<pre>
<h1>Build Log</h1>
<h3>
--------------------Configuration: UPnP Light - Win32 (WCE ARM) Release--------------------
</h3>
<h3>Command Lines</h3>
Creating command line "rc.exe /l 0x409 /fo"ARMRel/UPnP Light.res" /d UNDER_CE=300 /d _WIN32_WCE=300 /d "UNICODE" /d "_UNICODE" /d "NDEBUG" /d "WIN32_PLATFORM_PSPC=310" /d "ARM" /d "_ARM_" /r "C:\MicroStack\Light Bulb Device\PocketPC\UPnP Light.rc"" 
Creating temporary file "C:\DOCUME~1\YSAINTHI.AMR\LOCALS~1\Temp\RSP151.tmp" with contents
[
/nologo /W3 /D _WIN32_WCE=300 /D "WIN32_PLATFORM_PSPC=310" /D "ARM" /D "_ARM_" /D UNDER_CE=300 /D "UNICODE" /D "_UNICODE" /D "NDEBUG" /FR"ARMRel/" /Fo"ARMRel/" /Oxs /MC /c 
"C:\MicroStack\Light Bulb Device\PocketPC\ILibHTTPClient.c"
"C:\MicroStack\Light Bulb Device\PocketPC\ILibParsers.c"
"C:\MicroStack\Light Bulb Device\PocketPC\UPnP Light.cpp"
"C:\MicroStack\Light Bulb Device\PocketPC\UPnPMicroStack.c"
]
Creating command line "clarm.exe @C:\DOCUME~1\YSAINTHI.AMR\LOCALS~1\Temp\RSP151.tmp" 
Creating temporary file "C:\DOCUME~1\YSAINTHI.AMR\LOCALS~1\Temp\RSP152.tmp" with contents
[
/nologo /W3 /D _WIN32_WCE=300 /D "WIN32_PLATFORM_PSPC=310" /D "ARM" /D "_ARM_" /D UNDER_CE=300 /D "UNICODE" /D "_UNICODE" /D "NDEBUG" /FR"ARMRel/" /Fp"ARMRel/UPnP Light.pch" /Yc"stdafx.h" /Fo"ARMRel/" /Oxs /MC /c 
"C:\MicroStack\Light Bulb Device\PocketPC\StdAfx.cpp"
]
Creating command line "clarm.exe @C:\DOCUME~1\YSAINTHI.AMR\LOCALS~1\Temp\RSP152.tmp" 
Creating temporary file "C:\DOCUME~1\YSAINTHI.AMR\LOCALS~1\Temp\RSP153.tmp" with contents
[
commctrl.lib coredll.lib aygshell.lib /nologo /base:"0x00010000" /stack:0x10000,0x1000 /entry:"WinMainCRTStartup" /incremental:no /pdb:"ARMRel/Micro Light.pdb" /nodefaultlib:"libc.lib /nodefaultlib:libcd.lib /nodefaultlib:libcmt.lib /nodefaultlib:libcmtd.lib /nodefaultlib:msvcrt.lib /nodefaultlib:msvcrtd.lib /nodefaultlib:oldnames.lib" /out:"ARMRel/Micro Light.exe" /subsystem:windowsce,3.00 /align:"4096" /MACHINE:ARM 
".\ARMRel\ILibHTTPClient.obj"
".\ARMRel\ILibParsers.obj"
".\ARMRel\StdAfx.obj"
".\ARMRel\UPnP Light.obj"
".\ARMRel\UPnPMicroStack.obj"
".\ARMRel\UPnP Light.res"
"..\lib\winsock_x86.lib"
"..\lib\winsock_arm.lib"
]
Creating command line "link.exe @C:\DOCUME~1\YSAINTHI.AMR\LOCALS~1\Temp\RSP153.tmp"
<h3>Output Window</h3>
Compiling resources...
Compiling...
StdAfx.cpp
Compiling...
ILibHTTPClient.c
ILibParsers.c
Generating Code...
Compiling...
UPnP Light.cpp
Generating Code...
Compiling...
UPnPMicroStack.c
Generating Code...
Linking...
Creating command line "bscmake.exe /nologo /o"ARMRel/UPnP Light.bsc"  ".\ARMRel\StdAfx.sbr" ".\ARMRel\ILibHTTPClient.sbr" ".\ARMRel\ILibParsers.sbr" ".\ARMRel\UPnP Light.sbr" ".\ARMRel\UPnPMicroStack.sbr""
Creating browse info file...
<h3>Output Window</h3>



<h3>Results</h3>
Micro Light.exe - 0 error(s), 0 warning(s)
</pre>
</body>
</html>
