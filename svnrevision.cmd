@echo off
echo //%time% %date%>svntemp5674564.txt
for /F "skip=1 tokens=1*" %%i in (SVNRevision.h) do echo %%i %%j>>svntemp5674564.txt
type svntemp5674564.txt>SVNRevision.h
del svntemp5674564.txt