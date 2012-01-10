<html>
<body>
<pre>
<h1>Build Log</h1>
<h3>
--------------------Configuration: Renderer - Win32 (WCE ARM) Release--------------------
</h3>
<h3>Command Lines</h3>
Creating command line "rc.exe /l 0x409 /fo"ARMRel/MicroMediaRendererApp.res" /d UNDER_CE=300 /d _WIN32_WCE=300 /d "UNICODE" /d "_UNICODE" /d "NDEBUG" /d "WIN32_PLATFORM_PSPC=310" /d "ARM" /d "_ARM_" /r "C:\UPnPEmbeddedTools\MicroAvRenderer\PocketPC_MmrRslMp\MicroMediaRendererApp.rc"" 
Creating temporary file "C:\DOCUME~1\YSAINTHI.AMR\LOCALS~1\Temp\RSP146.tmp" with contents
[
/nologo /W3 /I "..\PocketPC_MmrRslMp" /I "..\DeviceBuilder\PocketPC" /I "..\\" /D _WIN32_WCE=300 /D "WIN32_PLATFORM_PSPC=310" /D "ARM" /D "_ARM_" /D UNDER_CE=300 /D "UNICODE" /D "_UNICODE" /D "NDEBUG" /D "_M3U_BUFFER" /D "MICROSTACK_NO_STDAFX" /FR"ARMRel/" /Fo"ARMRel/" /Oxs /MC /c 
"C:\UPnPEmbeddedTools\MicroAvRenderer\PocketPC_MmrRslMp\CMicroMediaRenderer.cpp"
"C:\UPnPEmbeddedTools\MicroAvRenderer\HttpPlaylistParser.c"
"C:\UPnPEmbeddedTools\MicroAvRenderer\DeviceBuilder\PocketPC\ILibParsers.c"
"C:\UPnPEmbeddedTools\MicroAvRenderer\MediaPlayerVersions_Methods.cpp"
"C:\UPnPEmbeddedTools\MicroAvRenderer\MicroMediaRenderer.c"
"C:\UPnPEmbeddedTools\MicroAvRenderer\PocketPC_MmrRslMp\MicroMediaRendererApp.cpp"
"C:\UPnPEmbeddedTools\MicroAvRenderer\MyString.c"
"C:\UPnPEmbeddedTools\MicroAvRenderer\RendererStateLogic.c"
"C:\UPnPEmbeddedTools\MicroAvRenderer\PocketPC_MmrRslMp\stdafx.cpp"
"C:\UPnPEmbeddedTools\MicroAvRenderer\DeviceBuilder\PocketPC\UpnpMicroStack.c"
]
Creating command line "clarm.exe @C:\DOCUME~1\YSAINTHI.AMR\LOCALS~1\Temp\RSP146.tmp" 
Creating temporary file "C:\DOCUME~1\YSAINTHI.AMR\LOCALS~1\Temp\RSP147.tmp" with contents
[
/nologo /W3 /I "..\PocketPC_MmrRslMp" /I "..\DeviceBuilder\PocketPC" /I "..\\" /D _WIN32_WCE=300 /D "WIN32_PLATFORM_PSPC=310" /D "ARM" /D "_ARM_" /D UNDER_CE=300 /D "UNICODE" /D "_UNICODE" /D "NDEBUG" /D "_M3U_BUFFER" /FR"ARMRel/" /Fo"ARMRel/" /Oxs /MC /c 
"C:\UPnPEmbeddedTools\MicroAvRenderer\DeviceBuilder\PocketPC\ILibAsyncServerSocket.c"
"C:\UPnPEmbeddedTools\MicroAvRenderer\DeviceBuilder\PocketPC\ILibAsyncSocket.c"
"C:\UPnPEmbeddedTools\MicroAvRenderer\DeviceBuilder\PocketPC\ILibHTTPClient.c"
"C:\UPnPEmbeddedTools\MicroAvRenderer\DeviceBuilder\PocketPC\ILibWebClient.c"
"C:\UPnPEmbeddedTools\MicroAvRenderer\DeviceBuilder\PocketPC\ILibWebServer.c"
]
Creating command line "clarm.exe @C:\DOCUME~1\YSAINTHI.AMR\LOCALS~1\Temp\RSP147.tmp" 
Creating temporary file "C:\DOCUME~1\YSAINTHI.AMR\LOCALS~1\Temp\RSP148.tmp" with contents
[
commctrl.lib coredll.lib aygshell.lib atlce300.lib /nologo /base:"0x00010000" /stack:0x10000,0x1000 /entry:"WinMainCRTStartup" /incremental:no /pdb:"ARMRel/Media Renderer.pdb" /nodefaultlib:"libc.lib /nodefaultlib:libcd.lib /nodefaultlib:libcmt.lib /nodefaultlib:libcmtd.lib /nodefaultlib:msvcrt.lib /nodefaultlib:msvcrtd.lib /nodefaultlib:oldnames.lib" /out:"ARMRel/Media Renderer.exe" /subsystem:windowsce,3.00 /align:"4096" /MACHINE:ARM 
.\ARMRel\CMicroMediaRenderer.obj
.\ARMRel\HttpPlaylistParser.obj
.\ARMRel\ILibParsers.obj
.\ARMRel\MediaPlayerVersions_Methods.obj
.\ARMRel\MicroMediaRenderer.obj
.\ARMRel\MicroMediaRendererApp.obj
.\ARMRel\MyString.obj
.\ARMRel\RendererStateLogic.obj
.\ARMRel\stdafx.obj
.\ARMRel\UpnpMicroStack.obj
.\ARMRel\MicroMediaRendererApp.res
.\winsock_arm.lib
.\winsock_x86.lib
.\ARMRel\ILibAsyncServerSocket.obj
.\ARMRel\ILibAsyncSocket.obj
.\ARMRel\ILibHTTPClient.obj
.\ARMRel\ILibWebClient.obj
.\ARMRel\ILibWebServer.obj
]
Creating command line "link.exe @C:\DOCUME~1\YSAINTHI.AMR\LOCALS~1\Temp\RSP148.tmp"
<h3>Output Window</h3>
Compiling resources...
Compiling...
CMicroMediaRenderer.cpp
Generating Code...
Compiling...
HttpPlaylistParser.c
ILibParsers.c
Generating Code...
C:\UPnPEmbeddedTools\MicroAvRenderer\HttpPlaylistParser.c(1785) : warning C4761: integral size mismatch in argument; conversion supplied
Compiling...
MediaPlayerVersions_Methods.cpp
Generating Code...
Compiling...
MicroMediaRenderer.c
Generating Code...
Compiling...
MicroMediaRendererApp.cpp
Generating Code...
Compiling...
MyString.c
RendererStateLogic.c
Generating Code...
Compiling...
stdafx.cpp
Generating Code...
Compiling...
UpnpMicroStack.c
Generating Code...
Compiling...
ILibAsyncServerSocket.c
ILibAsyncSocket.c
ILibHTTPClient.c
ILibWebClient.c
ILibWebServer.c
Generating Code...
Linking...



<h3>Results</h3>
Media Renderer.exe - 0 error(s), 1 warning(s)
</pre>
</body>
</html>
