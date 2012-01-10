<html>
<body>
<pre>
<h1>Build Log</h1>
<h3>
--------------------Configuration: PPC Media Server - Win32 (WCE ARM) Release--------------------
</h3>
<h3>Command Lines</h3>
Creating command line "rc.exe /l 0x409 /fo"ARMRel/PPC Media Server.res" /d UNDER_CE=300 /d _WIN32_WCE=300 /d "UNICODE" /d "_UNICODE" /d "NDEBUG" /d "WIN32_PLATFORM_PSPC=310" /d "ARM" /d "_ARM_" /r "C:\cygwin\home\Administrator\Micro Av Server\PocketPC\PPC Media Server.rc"" 
Creating temporary file "C:\DOCUME~1\nkidd\LOCALS~1\Temp\RSP280.tmp" with contents
[
/nologo /W3 /I "..\\" /I "..\DeviceBuilder\PocketPC" /D _WIN32_WCE=300 /D "WIN32_PLATFORM_PSPC=310" /D "ARM" /D "_ARM_" /D UNDER_CE=300 /D "UNICODE" /D "_UNICODE" /D "NDEBUG" /Fo"ARMRel/" /Oxs /MC /c 
"C:\cygwin\home\Administrator\Micro Av Server\CdsMediaClass.c"
"C:\cygwin\home\Administrator\Micro Av Server\CdsMediaObject.c"
"C:\cygwin\home\Administrator\Micro Av Server\CdsObjectToDidl.c"
"C:\cygwin\home\Administrator\Micro Av Server\DeviceBuilder\PocketPC\ILibAsyncServerSocket.c"
"C:\cygwin\home\Administrator\Micro Av Server\DeviceBuilder\PocketPC\ILibAsyncSocket.c"
"C:\cygwin\home\Administrator\Micro Av Server\DeviceBuilder\PocketPC\ILibParsers.c"
"C:\cygwin\home\Administrator\Micro Av Server\DeviceBuilder\PocketPC\ILibWebClient.c"
"C:\cygwin\home\Administrator\Micro Av Server\DeviceBuilder\PocketPC\ILibWebServer.c"
"C:\cygwin\home\Administrator\Micro Av Server\MicroMediaServer.c"
"C:\cygwin\home\Administrator\Micro Av Server\MimeTypes.c"
"C:\cygwin\home\Administrator\Micro Av Server\mystring.c"
"C:\cygwin\home\Administrator\Micro Av Server\PortingFunctions.c"
"C:\cygwin\home\Administrator\Micro Av Server\PocketPC\PPC Media Server.cpp"
"C:\cygwin\home\Administrator\Micro Av Server\DeviceBuilder\PocketPC\UpnpMicroStack.c"
]
Creating command line "clarm.exe @C:\DOCUME~1\nkidd\LOCALS~1\Temp\RSP280.tmp" 
Creating temporary file "C:\DOCUME~1\nkidd\LOCALS~1\Temp\RSP281.tmp" with contents
[
commctrl.lib coredll.lib aygshell.lib /nologo /base:"0x00010000" /stack:0x10000,0x1000 /entry:"WinMainCRTStartup" /incremental:no /pdb:"ARMRel/Media Server.pdb" /nodefaultlib:"libc.lib /nodefaultlib:libcd.lib /nodefaultlib:libcmt.lib /nodefaultlib:libcmtd.lib /nodefaultlib:msvcrt.lib /nodefaultlib:msvcrtd.lib /nodefaultlib:oldnames.lib" /out:"ARMRel/Media Server.exe" /subsystem:windowsce,3.00 /align:"4096" /MACHINE:ARM 
".\ARMRel\CdsMediaClass.obj"
".\ARMRel\CdsMediaObject.obj"
".\ARMRel\CdsObjectToDidl.obj"
".\ARMRel\ILibAsyncServerSocket.obj"
".\ARMRel\ILibAsyncSocket.obj"
".\ARMRel\ILibParsers.obj"
".\ARMRel\ILibWebClient.obj"
".\ARMRel\ILibWebServer.obj"
".\ARMRel\MicroMediaServer.obj"
".\ARMRel\MimeTypes.obj"
".\ARMRel\mystring.obj"
".\ARMRel\PortingFunctions.obj"
".\ARMRel\PPC Media Server.obj"
".\ARMRel\UpnpMicroStack.obj"
".\ARMRel\PPC Media Server.res"
".\winsock_x86.lib"
".\winsock_arm.lib"
]
Creating command line "link.exe @C:\DOCUME~1\nkidd\LOCALS~1\Temp\RSP281.tmp"
<h3>Output Window</h3>
Compiling resources...
Compiling...
CdsMediaClass.c
CdsMediaObject.c
CdsObjectToDidl.c
ILibAsyncServerSocket.c
ILibAsyncSocket.c
ILibParsers.c
ILibWebClient.c
ILibWebServer.c
MicroMediaServer.c
MimeTypes.c
mystring.c
PortingFunctions.c
Generating Code...
Compiling...
PPC Media Server.cpp
Generating Code...
Compiling...
UpnpMicroStack.c
Generating Code...
Linking...



<h3>Results</h3>
Media Server.exe - 0 error(s), 0 warning(s)
</pre>
</body>
</html>
