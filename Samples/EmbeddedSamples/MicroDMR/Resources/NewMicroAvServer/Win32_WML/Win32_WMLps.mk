
Win32_WMLps.dll: dlldata.obj Win32_WML_p.obj Win32_WML_i.obj
	link /dll /out:Win32_WMLps.dll /def:Win32_WMLps.def /entry:DllMain dlldata.obj Win32_WML_p.obj Win32_WML_i.obj \
		kernel32.lib rpcndr.lib rpcns4.lib rpcrt4.lib oleaut32.lib uuid.lib \
.c.obj:
	cl /c /Ox /DWIN32 /D_WIN32_WINNT=0x0400 /DREGISTER_PROXY_DLL \
		$<

clean:
	@del Win32_WMLps.dll
	@del Win32_WMLps.lib
	@del Win32_WMLps.exp
	@del dlldata.obj
	@del Win32_WML_p.obj
	@del Win32_WML_i.obj
