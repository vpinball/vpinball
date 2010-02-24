
VBATestps.dll: dlldata.obj VBATest_p.obj VBATest_i.obj
	link /dll /out:VBATestps.dll /def:VBATestps.def /entry:DllMain dlldata.obj VBATest_p.obj VBATest_i.obj \
		kernel32.lib rpcndr.lib rpcns4.lib rpcrt4.lib oleaut32.lib uuid.lib \

.c.obj:
	cl /c /Ox /DWIN32 /D_WIN32_WINNT=0x0400 /DREGISTER_PROXY_DLL \
		$<

clean:
	@del VBATestps.dll
	@del VBATestps.lib
	@del VBATestps.exp
	@del dlldata.obj
	@del VBATest_p.obj
	@del VBATest_i.obj
