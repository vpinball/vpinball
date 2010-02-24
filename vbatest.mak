# Microsoft Developer Studio Generated NMAKE File, Based on VBATest.dsp
!IF "$(CFG)" == ""
CFG=VBATest - Win32 Debug
!MESSAGE No configuration specified. Defaulting to VBATest - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "VBATest - Win32 Debug" && "$(CFG)" != "VBATest - Win32 Unicode Debug" && "$(CFG)" != "VBATest - Win32 Release MinSize" && "$(CFG)" != "VBATest - Win32 Release MinDependency" && "$(CFG)" != "VBATest - Win32 Unicode Release MinSize" && "$(CFG)" != "VBATest - Win32 Unicode Release MinDependency"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "VBATest.mak" CFG="VBATest - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "VBATest - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "VBATest - Win32 Unicode Debug" (based on "Win32 (x86) Application")
!MESSAGE "VBATest - Win32 Release MinSize" (based on "Win32 (x86) Application")
!MESSAGE "VBATest - Win32 Release MinDependency" (based on "Win32 (x86) Application")
!MESSAGE "VBATest - Win32 Unicode Release MinSize" (based on "Win32 (x86) Application")
!MESSAGE "VBATest - Win32 Unicode Release MinDependency" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "VBATest - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\VBATest.exe" ".\VBATest.tlb" ".\VBATest.h" ".\VBATest_i.c" ".\Debug\regsvr32.trg"


CLEAN :
	-@erase "$(INTDIR)\Ball.obj"
	-@erase "$(INTDIR)\Bumper.obj"
	-@erase "$(INTDIR)\collide.obj"
	-@erase "$(INTDIR)\Decal.obj"
	-@erase "$(INTDIR)\DragPoint.obj"
	-@erase "$(INTDIR)\draw.obj"
	-@erase "$(INTDIR)\extern.obj"
	-@erase "$(INTDIR)\Flipper.obj"
	-@erase "$(INTDIR)\Gate.obj"
	-@erase "$(INTDIR)\hitflipper.obj"
	-@erase "$(INTDIR)\HitPlunger.obj"
	-@erase "$(INTDIR)\HitSur.obj"
	-@erase "$(INTDIR)\IEditable.obj"
	-@erase "$(INTDIR)\ISelect.obj"
	-@erase "$(INTDIR)\Kicker.obj"
	-@erase "$(INTDIR)\Light.obj"
	-@erase "$(INTDIR)\lzwreader.obj"
	-@erase "$(INTDIR)\lzwwriter.obj"
	-@erase "$(INTDIR)\Mesh.obj"
	-@erase "$(INTDIR)\OLEAUTO.OBJ"
	-@erase "$(INTDIR)\PaintSur.obj"
	-@erase "$(INTDIR)\Pin3D.obj"
	-@erase "$(INTDIR)\PinImage.obj"
	-@erase "$(INTDIR)\PinSound.obj"
	-@erase "$(INTDIR)\PinTable.obj"
	-@erase "$(INTDIR)\PinUndo.obj"
	-@erase "$(INTDIR)\Player.obj"
	-@erase "$(INTDIR)\Plunger.obj"
	-@erase "$(INTDIR)\Spinner.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\Sur.obj"
	-@erase "$(INTDIR)\Surface.obj"
	-@erase "$(INTDIR)\Textbox.obj"
	-@erase "$(INTDIR)\Timer.obj"
	-@erase "$(INTDIR)\Trigger.obj"
	-@erase "$(INTDIR)\VBATest.obj"
	-@erase "$(INTDIR)\VBATest.pch"
	-@erase "$(INTDIR)\VBATest.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\VPinball.obj"
	-@erase "$(INTDIR)\wall.obj"
	-@erase "$(INTDIR)\wavread.obj"
	-@erase "$(OUTDIR)\VBATest.exe"
	-@erase "$(OUTDIR)\VBATest.ilk"
	-@erase "$(OUTDIR)\VBATest.pdb"
	-@erase ".\VBATest.h"
	-@erase ".\VBATest.tlb"
	-@erase ".\VBATest_i.c"
	-@erase ".\Debug\regsvr32.trg"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MLd /W3 /Gm /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /Fp"$(INTDIR)\VBATest.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=
RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\VBATest.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\VBATest.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib ddraw.lib dxguid.lib dsound.lib winmm.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\VBATest.pdb" /debug /machine:I386 /out:"$(OUTDIR)\VBATest.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\Ball.obj" \
	"$(INTDIR)\Bumper.obj" \
	"$(INTDIR)\collide.obj" \
	"$(INTDIR)\Decal.obj" \
	"$(INTDIR)\DragPoint.obj" \
	"$(INTDIR)\draw.obj" \
	"$(INTDIR)\extern.obj" \
	"$(INTDIR)\Flipper.obj" \
	"$(INTDIR)\Gate.obj" \
	"$(INTDIR)\hitflipper.obj" \
	"$(INTDIR)\HitPlunger.obj" \
	"$(INTDIR)\HitSur.obj" \
	"$(INTDIR)\IEditable.obj" \
	"$(INTDIR)\ISelect.obj" \
	"$(INTDIR)\Kicker.obj" \
	"$(INTDIR)\Light.obj" \
	"$(INTDIR)\lzwreader.obj" \
	"$(INTDIR)\lzwwriter.obj" \
	"$(INTDIR)\Mesh.obj" \
	"$(INTDIR)\OLEAUTO.OBJ" \
	"$(INTDIR)\PaintSur.obj" \
	"$(INTDIR)\Pin3D.obj" \
	"$(INTDIR)\PinImage.obj" \
	"$(INTDIR)\PinSound.obj" \
	"$(INTDIR)\PinTable.obj" \
	"$(INTDIR)\PinUndo.obj" \
	"$(INTDIR)\Player.obj" \
	"$(INTDIR)\Plunger.obj" \
	"$(INTDIR)\Spinner.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\Sur.obj" \
	"$(INTDIR)\Surface.obj" \
	"$(INTDIR)\Textbox.obj" \
	"$(INTDIR)\Timer.obj" \
	"$(INTDIR)\Trigger.obj" \
	"$(INTDIR)\VBATest.obj" \
	"$(INTDIR)\VPinball.obj" \
	"$(INTDIR)\wall.obj" \
	"$(INTDIR)\wavread.obj" \
	"$(INTDIR)\VBATest.res"

"$(OUTDIR)\VBATest.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

OutDir=.\Debug
TargetPath=.\Debug\VBATest.exe
InputPath=.\Debug\VBATest.exe
SOURCE="$(InputPath)"

"$(OUTDIR)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	"$(TargetPath)" /RegServer 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
	echo Server registration done! 
<< 
	

!ELSEIF  "$(CFG)" == "VBATest - Win32 Unicode Debug"

OUTDIR=.\DebugU
INTDIR=.\DebugU

ALL : ".\VPinball.exe" ".\VBATest.tlb" ".\VBATest.h" ".\VBATest_i.c" ".\DebugU\regsvr32.trg"


CLEAN :
	-@erase "$(INTDIR)\Ball.obj"
	-@erase "$(INTDIR)\Bumper.obj"
	-@erase "$(INTDIR)\collide.obj"
	-@erase "$(INTDIR)\Decal.obj"
	-@erase "$(INTDIR)\DragPoint.obj"
	-@erase "$(INTDIR)\draw.obj"
	-@erase "$(INTDIR)\extern.obj"
	-@erase "$(INTDIR)\Flipper.obj"
	-@erase "$(INTDIR)\Gate.obj"
	-@erase "$(INTDIR)\hitflipper.obj"
	-@erase "$(INTDIR)\HitPlunger.obj"
	-@erase "$(INTDIR)\HitSur.obj"
	-@erase "$(INTDIR)\IEditable.obj"
	-@erase "$(INTDIR)\ISelect.obj"
	-@erase "$(INTDIR)\Kicker.obj"
	-@erase "$(INTDIR)\Light.obj"
	-@erase "$(INTDIR)\lzwreader.obj"
	-@erase "$(INTDIR)\lzwwriter.obj"
	-@erase "$(INTDIR)\Mesh.obj"
	-@erase "$(INTDIR)\OLEAUTO.OBJ"
	-@erase "$(INTDIR)\PaintSur.obj"
	-@erase "$(INTDIR)\Pin3D.obj"
	-@erase "$(INTDIR)\PinImage.obj"
	-@erase "$(INTDIR)\PinSound.obj"
	-@erase "$(INTDIR)\PinTable.obj"
	-@erase "$(INTDIR)\PinUndo.obj"
	-@erase "$(INTDIR)\Player.obj"
	-@erase "$(INTDIR)\Plunger.obj"
	-@erase "$(INTDIR)\Spinner.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\Sur.obj"
	-@erase "$(INTDIR)\Surface.obj"
	-@erase "$(INTDIR)\Textbox.obj"
	-@erase "$(INTDIR)\Timer.obj"
	-@erase "$(INTDIR)\Trigger.obj"
	-@erase "$(INTDIR)\VBATest.obj"
	-@erase "$(INTDIR)\VBATest.pch"
	-@erase "$(INTDIR)\VBATest.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\VPinball.obj"
	-@erase "$(INTDIR)\wall.obj"
	-@erase "$(INTDIR)\wavread.obj"
	-@erase "$(OUTDIR)\VPinball.pdb"
	-@erase ".\VBATest.h"
	-@erase ".\VBATest.tlb"
	-@erase ".\VBATest_i.c"
	-@erase ".\VPinball.exe"
	-@erase ".\VPinball.ilk"
	-@erase ".\DebugU\regsvr32.trg"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MLd /W3 /Gm /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_UNICODE" /Fp"$(INTDIR)\VBATest.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=
RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\VBATest.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\VBATest.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /entry:"wWinMainCRTStartup" /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\VPinball.pdb" /debug /machine:I386 /out:"VPinball.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\Ball.obj" \
	"$(INTDIR)\Bumper.obj" \
	"$(INTDIR)\collide.obj" \
	"$(INTDIR)\Decal.obj" \
	"$(INTDIR)\DragPoint.obj" \
	"$(INTDIR)\draw.obj" \
	"$(INTDIR)\extern.obj" \
	"$(INTDIR)\Flipper.obj" \
	"$(INTDIR)\Gate.obj" \
	"$(INTDIR)\hitflipper.obj" \
	"$(INTDIR)\HitPlunger.obj" \
	"$(INTDIR)\HitSur.obj" \
	"$(INTDIR)\IEditable.obj" \
	"$(INTDIR)\ISelect.obj" \
	"$(INTDIR)\Kicker.obj" \
	"$(INTDIR)\Light.obj" \
	"$(INTDIR)\lzwreader.obj" \
	"$(INTDIR)\lzwwriter.obj" \
	"$(INTDIR)\Mesh.obj" \
	"$(INTDIR)\OLEAUTO.OBJ" \
	"$(INTDIR)\PaintSur.obj" \
	"$(INTDIR)\Pin3D.obj" \
	"$(INTDIR)\PinImage.obj" \
	"$(INTDIR)\PinSound.obj" \
	"$(INTDIR)\PinTable.obj" \
	"$(INTDIR)\PinUndo.obj" \
	"$(INTDIR)\Player.obj" \
	"$(INTDIR)\Plunger.obj" \
	"$(INTDIR)\Spinner.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\Sur.obj" \
	"$(INTDIR)\Surface.obj" \
	"$(INTDIR)\Textbox.obj" \
	"$(INTDIR)\Timer.obj" \
	"$(INTDIR)\Trigger.obj" \
	"$(INTDIR)\VBATest.obj" \
	"$(INTDIR)\VPinball.obj" \
	"$(INTDIR)\wall.obj" \
	"$(INTDIR)\wavread.obj" \
	"$(INTDIR)\VBATest.res"

".\VPinball.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

OutDir=.\DebugU
TargetPath=.\VPinball.exe
InputPath=.\VPinball.exe
SOURCE="$(InputPath)"

"$(OUTDIR)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	if "%OS%"=="" goto NOTNT 
	if not "%OS%"=="Windows_NT" goto NOTNT 
	"$(TargetPath)" /RegServer 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
	echo Server registration done! 
	goto end 
	:NOTNT 
	echo Warning : Cannot register Unicode EXE on Windows 95 
	:end 
<< 
	

!ELSEIF  "$(CFG)" == "VBATest - Win32 Release MinSize"

OUTDIR=.\ReleaseMinSize
INTDIR=.\ReleaseMinSize

ALL : ".\VPinball.exe" ".\VBATest.tlb" ".\VBATest.h" ".\VBATest_i.c" ".\ReleaseMinSize\regsvr32.trg"


CLEAN :
	-@erase "$(INTDIR)\Ball.obj"
	-@erase "$(INTDIR)\Bumper.obj"
	-@erase "$(INTDIR)\collide.obj"
	-@erase "$(INTDIR)\Decal.obj"
	-@erase "$(INTDIR)\DragPoint.obj"
	-@erase "$(INTDIR)\draw.obj"
	-@erase "$(INTDIR)\extern.obj"
	-@erase "$(INTDIR)\Flipper.obj"
	-@erase "$(INTDIR)\Gate.obj"
	-@erase "$(INTDIR)\hitflipper.obj"
	-@erase "$(INTDIR)\HitPlunger.obj"
	-@erase "$(INTDIR)\HitSur.obj"
	-@erase "$(INTDIR)\IEditable.obj"
	-@erase "$(INTDIR)\ISelect.obj"
	-@erase "$(INTDIR)\Kicker.obj"
	-@erase "$(INTDIR)\Light.obj"
	-@erase "$(INTDIR)\lzwreader.obj"
	-@erase "$(INTDIR)\lzwwriter.obj"
	-@erase "$(INTDIR)\Mesh.obj"
	-@erase "$(INTDIR)\OLEAUTO.OBJ"
	-@erase "$(INTDIR)\PaintSur.obj"
	-@erase "$(INTDIR)\Pin3D.obj"
	-@erase "$(INTDIR)\PinImage.obj"
	-@erase "$(INTDIR)\PinSound.obj"
	-@erase "$(INTDIR)\PinTable.obj"
	-@erase "$(INTDIR)\PinUndo.obj"
	-@erase "$(INTDIR)\Player.obj"
	-@erase "$(INTDIR)\Plunger.obj"
	-@erase "$(INTDIR)\Spinner.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\Sur.obj"
	-@erase "$(INTDIR)\Surface.obj"
	-@erase "$(INTDIR)\Textbox.obj"
	-@erase "$(INTDIR)\Timer.obj"
	-@erase "$(INTDIR)\Trigger.obj"
	-@erase "$(INTDIR)\VBATest.obj"
	-@erase "$(INTDIR)\VBATest.pch"
	-@erase "$(INTDIR)\VBATest.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\VPinball.obj"
	-@erase "$(INTDIR)\wall.obj"
	-@erase "$(INTDIR)\wavread.obj"
	-@erase ".\VBATest.h"
	-@erase ".\VBATest.tlb"
	-@erase ".\VBATest_i.c"
	-@erase ".\VPinball.exe"
	-@erase ".\ReleaseMinSize\regsvr32.trg"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /ML /W3 /O1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_ATL_DLL" /Fp"$(INTDIR)\VBATest.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=
RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\VBATest.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\VBATest.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\VPinball.pdb" /machine:I386 /out:"VPinball.exe" 
LINK32_OBJS= \
	"$(INTDIR)\Ball.obj" \
	"$(INTDIR)\Bumper.obj" \
	"$(INTDIR)\collide.obj" \
	"$(INTDIR)\Decal.obj" \
	"$(INTDIR)\DragPoint.obj" \
	"$(INTDIR)\draw.obj" \
	"$(INTDIR)\extern.obj" \
	"$(INTDIR)\Flipper.obj" \
	"$(INTDIR)\Gate.obj" \
	"$(INTDIR)\hitflipper.obj" \
	"$(INTDIR)\HitPlunger.obj" \
	"$(INTDIR)\HitSur.obj" \
	"$(INTDIR)\IEditable.obj" \
	"$(INTDIR)\ISelect.obj" \
	"$(INTDIR)\Kicker.obj" \
	"$(INTDIR)\Light.obj" \
	"$(INTDIR)\lzwreader.obj" \
	"$(INTDIR)\lzwwriter.obj" \
	"$(INTDIR)\Mesh.obj" \
	"$(INTDIR)\OLEAUTO.OBJ" \
	"$(INTDIR)\PaintSur.obj" \
	"$(INTDIR)\Pin3D.obj" \
	"$(INTDIR)\PinImage.obj" \
	"$(INTDIR)\PinSound.obj" \
	"$(INTDIR)\PinTable.obj" \
	"$(INTDIR)\PinUndo.obj" \
	"$(INTDIR)\Player.obj" \
	"$(INTDIR)\Plunger.obj" \
	"$(INTDIR)\Spinner.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\Sur.obj" \
	"$(INTDIR)\Surface.obj" \
	"$(INTDIR)\Textbox.obj" \
	"$(INTDIR)\Timer.obj" \
	"$(INTDIR)\Trigger.obj" \
	"$(INTDIR)\VBATest.obj" \
	"$(INTDIR)\VPinball.obj" \
	"$(INTDIR)\wall.obj" \
	"$(INTDIR)\wavread.obj" \
	"$(INTDIR)\VBATest.res"

".\VPinball.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

OutDir=.\ReleaseMinSize
TargetPath=.\VPinball.exe
InputPath=.\VPinball.exe
SOURCE="$(InputPath)"

"$(OUTDIR)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	"$(TargetPath)" /RegServer 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
	echo Server registration done! 
<< 
	

!ELSEIF  "$(CFG)" == "VBATest - Win32 Release MinDependency"

OUTDIR=.\ReleaseMinDependency
INTDIR=.\ReleaseMinDependency
# Begin Custom Macros
OutDir=.\ReleaseMinDependency
# End Custom Macros

ALL : "$(OUTDIR)\VBATest.exe" ".\ReleaseMinDependency\regsvr32.trg"


CLEAN :
	-@erase "$(INTDIR)\Ball.obj"
	-@erase "$(INTDIR)\Bumper.obj"
	-@erase "$(INTDIR)\collide.obj"
	-@erase "$(INTDIR)\Decal.obj"
	-@erase "$(INTDIR)\DragPoint.obj"
	-@erase "$(INTDIR)\draw.obj"
	-@erase "$(INTDIR)\extern.obj"
	-@erase "$(INTDIR)\Flipper.obj"
	-@erase "$(INTDIR)\Gate.obj"
	-@erase "$(INTDIR)\hitflipper.obj"
	-@erase "$(INTDIR)\HitPlunger.obj"
	-@erase "$(INTDIR)\HitSur.obj"
	-@erase "$(INTDIR)\IEditable.obj"
	-@erase "$(INTDIR)\ISelect.obj"
	-@erase "$(INTDIR)\Kicker.obj"
	-@erase "$(INTDIR)\Light.obj"
	-@erase "$(INTDIR)\lzwreader.obj"
	-@erase "$(INTDIR)\lzwwriter.obj"
	-@erase "$(INTDIR)\Mesh.obj"
	-@erase "$(INTDIR)\OLEAUTO.OBJ"
	-@erase "$(INTDIR)\PaintSur.obj"
	-@erase "$(INTDIR)\Pin3D.obj"
	-@erase "$(INTDIR)\PinImage.obj"
	-@erase "$(INTDIR)\PinSound.obj"
	-@erase "$(INTDIR)\PinTable.obj"
	-@erase "$(INTDIR)\PinUndo.obj"
	-@erase "$(INTDIR)\Player.obj"
	-@erase "$(INTDIR)\Plunger.obj"
	-@erase "$(INTDIR)\Spinner.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\Sur.obj"
	-@erase "$(INTDIR)\Surface.obj"
	-@erase "$(INTDIR)\Textbox.obj"
	-@erase "$(INTDIR)\Timer.obj"
	-@erase "$(INTDIR)\Trigger.obj"
	-@erase "$(INTDIR)\VBATest.obj"
	-@erase "$(INTDIR)\VBATest.pch"
	-@erase "$(INTDIR)\VBATest.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\VPinball.obj"
	-@erase "$(INTDIR)\wall.obj"
	-@erase "$(INTDIR)\wavread.obj"
	-@erase "$(OUTDIR)\VBATest.exe"
	-@erase "$(OUTDIR)\VBATest.pdb"
	-@erase ".\VBATest.h"
	-@erase ".\VBATest.tlb"
	-@erase ".\VBATest_i.c"
	-@erase ".\ReleaseMinDependency\regsvr32.trg"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /ML /W3 /Zi /O1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_ATL_STATIC_REGISTRY" /Fp"$(INTDIR)\VBATest.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=
RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\VBATest.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\VBATest.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib ddraw.lib dxguid.lib dsound.lib winmm.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\VBATest.pdb" /debug /machine:I386 /out:"$(OUTDIR)\VBATest.exe" /DEBUGTYPE:CV,FIXUP 
LINK32_OBJS= \
	"$(INTDIR)\Ball.obj" \
	"$(INTDIR)\Bumper.obj" \
	"$(INTDIR)\collide.obj" \
	"$(INTDIR)\Decal.obj" \
	"$(INTDIR)\DragPoint.obj" \
	"$(INTDIR)\draw.obj" \
	"$(INTDIR)\extern.obj" \
	"$(INTDIR)\Flipper.obj" \
	"$(INTDIR)\Gate.obj" \
	"$(INTDIR)\hitflipper.obj" \
	"$(INTDIR)\HitPlunger.obj" \
	"$(INTDIR)\HitSur.obj" \
	"$(INTDIR)\IEditable.obj" \
	"$(INTDIR)\ISelect.obj" \
	"$(INTDIR)\Kicker.obj" \
	"$(INTDIR)\Light.obj" \
	"$(INTDIR)\lzwreader.obj" \
	"$(INTDIR)\lzwwriter.obj" \
	"$(INTDIR)\Mesh.obj" \
	"$(INTDIR)\OLEAUTO.OBJ" \
	"$(INTDIR)\PaintSur.obj" \
	"$(INTDIR)\Pin3D.obj" \
	"$(INTDIR)\PinImage.obj" \
	"$(INTDIR)\PinSound.obj" \
	"$(INTDIR)\PinTable.obj" \
	"$(INTDIR)\PinUndo.obj" \
	"$(INTDIR)\Player.obj" \
	"$(INTDIR)\Plunger.obj" \
	"$(INTDIR)\Spinner.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\Sur.obj" \
	"$(INTDIR)\Surface.obj" \
	"$(INTDIR)\Textbox.obj" \
	"$(INTDIR)\Timer.obj" \
	"$(INTDIR)\Trigger.obj" \
	"$(INTDIR)\VBATest.obj" \
	"$(INTDIR)\VPinball.obj" \
	"$(INTDIR)\wall.obj" \
	"$(INTDIR)\wavread.obj" \
	"$(INTDIR)\VBATest.res"

"$(OUTDIR)\VBATest.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

OutDir=.\ReleaseMinDependency
TargetPath=.\ReleaseMinDependency\VBATest.exe
InputPath=.\ReleaseMinDependency\VBATest.exe
SOURCE="$(InputPath)"

"$(OUTDIR)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	"$(TargetPath)" /RegServer 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
	echo Server registration done! 
<< 
	
SOURCE="$(InputPath)"
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.\ReleaseMinDependency
# End Custom Macros

$(DS_POSTBUILD_DEP) : "$(OUTDIR)\VBATest.exe" ".\ReleaseMinDependency\regsvr32.trg"
   copy .\releasemindependency\vbatest.exe .\VPinball.exe
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ELSEIF  "$(CFG)" == "VBATest - Win32 Unicode Release MinSize"

OUTDIR=.\ReleaseUMinSize
INTDIR=.\ReleaseUMinSize

ALL : ".\VPinball.exe" ".\VBATest.tlb" ".\VBATest.h" ".\VBATest_i.c" ".\ReleaseUMinSize\regsvr32.trg"


CLEAN :
	-@erase "$(INTDIR)\Ball.obj"
	-@erase "$(INTDIR)\Bumper.obj"
	-@erase "$(INTDIR)\collide.obj"
	-@erase "$(INTDIR)\Decal.obj"
	-@erase "$(INTDIR)\DragPoint.obj"
	-@erase "$(INTDIR)\draw.obj"
	-@erase "$(INTDIR)\extern.obj"
	-@erase "$(INTDIR)\Flipper.obj"
	-@erase "$(INTDIR)\Gate.obj"
	-@erase "$(INTDIR)\hitflipper.obj"
	-@erase "$(INTDIR)\HitPlunger.obj"
	-@erase "$(INTDIR)\HitSur.obj"
	-@erase "$(INTDIR)\IEditable.obj"
	-@erase "$(INTDIR)\ISelect.obj"
	-@erase "$(INTDIR)\Kicker.obj"
	-@erase "$(INTDIR)\Light.obj"
	-@erase "$(INTDIR)\lzwreader.obj"
	-@erase "$(INTDIR)\lzwwriter.obj"
	-@erase "$(INTDIR)\Mesh.obj"
	-@erase "$(INTDIR)\OLEAUTO.OBJ"
	-@erase "$(INTDIR)\PaintSur.obj"
	-@erase "$(INTDIR)\Pin3D.obj"
	-@erase "$(INTDIR)\PinImage.obj"
	-@erase "$(INTDIR)\PinSound.obj"
	-@erase "$(INTDIR)\PinTable.obj"
	-@erase "$(INTDIR)\PinUndo.obj"
	-@erase "$(INTDIR)\Player.obj"
	-@erase "$(INTDIR)\Plunger.obj"
	-@erase "$(INTDIR)\Spinner.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\Sur.obj"
	-@erase "$(INTDIR)\Surface.obj"
	-@erase "$(INTDIR)\Textbox.obj"
	-@erase "$(INTDIR)\Timer.obj"
	-@erase "$(INTDIR)\Trigger.obj"
	-@erase "$(INTDIR)\VBATest.obj"
	-@erase "$(INTDIR)\VBATest.pch"
	-@erase "$(INTDIR)\VBATest.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\VPinball.obj"
	-@erase "$(INTDIR)\wall.obj"
	-@erase "$(INTDIR)\wavread.obj"
	-@erase ".\VBATest.h"
	-@erase ".\VBATest.tlb"
	-@erase ".\VBATest_i.c"
	-@erase ".\VPinball.exe"
	-@erase ".\ReleaseUMinSize\regsvr32.trg"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /ML /W3 /O1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_UNICODE" /D "_ATL_DLL" /D "_ATL_MIN_CRT" /Fp"$(INTDIR)\VBATest.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=
RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\VBATest.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\VBATest.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\VPinball.pdb" /machine:I386 /out:"VPinball.exe" 
LINK32_OBJS= \
	"$(INTDIR)\Ball.obj" \
	"$(INTDIR)\Bumper.obj" \
	"$(INTDIR)\collide.obj" \
	"$(INTDIR)\Decal.obj" \
	"$(INTDIR)\DragPoint.obj" \
	"$(INTDIR)\draw.obj" \
	"$(INTDIR)\extern.obj" \
	"$(INTDIR)\Flipper.obj" \
	"$(INTDIR)\Gate.obj" \
	"$(INTDIR)\hitflipper.obj" \
	"$(INTDIR)\HitPlunger.obj" \
	"$(INTDIR)\HitSur.obj" \
	"$(INTDIR)\IEditable.obj" \
	"$(INTDIR)\ISelect.obj" \
	"$(INTDIR)\Kicker.obj" \
	"$(INTDIR)\Light.obj" \
	"$(INTDIR)\lzwreader.obj" \
	"$(INTDIR)\lzwwriter.obj" \
	"$(INTDIR)\Mesh.obj" \
	"$(INTDIR)\OLEAUTO.OBJ" \
	"$(INTDIR)\PaintSur.obj" \
	"$(INTDIR)\Pin3D.obj" \
	"$(INTDIR)\PinImage.obj" \
	"$(INTDIR)\PinSound.obj" \
	"$(INTDIR)\PinTable.obj" \
	"$(INTDIR)\PinUndo.obj" \
	"$(INTDIR)\Player.obj" \
	"$(INTDIR)\Plunger.obj" \
	"$(INTDIR)\Spinner.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\Sur.obj" \
	"$(INTDIR)\Surface.obj" \
	"$(INTDIR)\Textbox.obj" \
	"$(INTDIR)\Timer.obj" \
	"$(INTDIR)\Trigger.obj" \
	"$(INTDIR)\VBATest.obj" \
	"$(INTDIR)\VPinball.obj" \
	"$(INTDIR)\wall.obj" \
	"$(INTDIR)\wavread.obj" \
	"$(INTDIR)\VBATest.res"

".\VPinball.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

OutDir=.\ReleaseUMinSize
TargetPath=.\VPinball.exe
InputPath=.\VPinball.exe
SOURCE="$(InputPath)"

"$(OUTDIR)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	if "%OS%"=="" goto NOTNT 
	if not "%OS%"=="Windows_NT" goto NOTNT 
	"$(TargetPath)" /RegServer 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
	echo Server registration done! 
	goto end 
	:NOTNT 
	echo Warning : Cannot register Unicode EXE on Windows 95 
	:end 
<< 
	

!ELSEIF  "$(CFG)" == "VBATest - Win32 Unicode Release MinDependency"

OUTDIR=.\ReleaseUMinDependency
INTDIR=.\ReleaseUMinDependency

ALL : ".\VPinball.exe" ".\VBATest.tlb" ".\VBATest.h" ".\VBATest_i.c" ".\ReleaseUMinDependency\regsvr32.trg"


CLEAN :
	-@erase "$(INTDIR)\Ball.obj"
	-@erase "$(INTDIR)\Bumper.obj"
	-@erase "$(INTDIR)\collide.obj"
	-@erase "$(INTDIR)\Decal.obj"
	-@erase "$(INTDIR)\DragPoint.obj"
	-@erase "$(INTDIR)\draw.obj"
	-@erase "$(INTDIR)\extern.obj"
	-@erase "$(INTDIR)\Flipper.obj"
	-@erase "$(INTDIR)\Gate.obj"
	-@erase "$(INTDIR)\hitflipper.obj"
	-@erase "$(INTDIR)\HitPlunger.obj"
	-@erase "$(INTDIR)\HitSur.obj"
	-@erase "$(INTDIR)\IEditable.obj"
	-@erase "$(INTDIR)\ISelect.obj"
	-@erase "$(INTDIR)\Kicker.obj"
	-@erase "$(INTDIR)\Light.obj"
	-@erase "$(INTDIR)\lzwreader.obj"
	-@erase "$(INTDIR)\lzwwriter.obj"
	-@erase "$(INTDIR)\Mesh.obj"
	-@erase "$(INTDIR)\OLEAUTO.OBJ"
	-@erase "$(INTDIR)\PaintSur.obj"
	-@erase "$(INTDIR)\Pin3D.obj"
	-@erase "$(INTDIR)\PinImage.obj"
	-@erase "$(INTDIR)\PinSound.obj"
	-@erase "$(INTDIR)\PinTable.obj"
	-@erase "$(INTDIR)\PinUndo.obj"
	-@erase "$(INTDIR)\Player.obj"
	-@erase "$(INTDIR)\Plunger.obj"
	-@erase "$(INTDIR)\Spinner.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\Sur.obj"
	-@erase "$(INTDIR)\Surface.obj"
	-@erase "$(INTDIR)\Textbox.obj"
	-@erase "$(INTDIR)\Timer.obj"
	-@erase "$(INTDIR)\Trigger.obj"
	-@erase "$(INTDIR)\VBATest.obj"
	-@erase "$(INTDIR)\VBATest.pch"
	-@erase "$(INTDIR)\VBATest.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\VPinball.obj"
	-@erase "$(INTDIR)\wall.obj"
	-@erase "$(INTDIR)\wavread.obj"
	-@erase ".\VBATest.h"
	-@erase ".\VBATest.tlb"
	-@erase ".\VBATest_i.c"
	-@erase ".\VPinball.exe"
	-@erase ".\ReleaseUMinDependency\regsvr32.trg"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /ML /W3 /O1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_UNICODE" /D "_ATL_STATIC_REGISTRY" /D "_ATL_MIN_CRT" /Fp"$(INTDIR)\VBATest.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=
RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\VBATest.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\VBATest.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\VPinball.pdb" /machine:I386 /out:"VPinball.exe" 
LINK32_OBJS= \
	"$(INTDIR)\Ball.obj" \
	"$(INTDIR)\Bumper.obj" \
	"$(INTDIR)\collide.obj" \
	"$(INTDIR)\Decal.obj" \
	"$(INTDIR)\DragPoint.obj" \
	"$(INTDIR)\draw.obj" \
	"$(INTDIR)\extern.obj" \
	"$(INTDIR)\Flipper.obj" \
	"$(INTDIR)\Gate.obj" \
	"$(INTDIR)\hitflipper.obj" \
	"$(INTDIR)\HitPlunger.obj" \
	"$(INTDIR)\HitSur.obj" \
	"$(INTDIR)\IEditable.obj" \
	"$(INTDIR)\ISelect.obj" \
	"$(INTDIR)\Kicker.obj" \
	"$(INTDIR)\Light.obj" \
	"$(INTDIR)\lzwreader.obj" \
	"$(INTDIR)\lzwwriter.obj" \
	"$(INTDIR)\Mesh.obj" \
	"$(INTDIR)\OLEAUTO.OBJ" \
	"$(INTDIR)\PaintSur.obj" \
	"$(INTDIR)\Pin3D.obj" \
	"$(INTDIR)\PinImage.obj" \
	"$(INTDIR)\PinSound.obj" \
	"$(INTDIR)\PinTable.obj" \
	"$(INTDIR)\PinUndo.obj" \
	"$(INTDIR)\Player.obj" \
	"$(INTDIR)\Plunger.obj" \
	"$(INTDIR)\Spinner.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\Sur.obj" \
	"$(INTDIR)\Surface.obj" \
	"$(INTDIR)\Textbox.obj" \
	"$(INTDIR)\Timer.obj" \
	"$(INTDIR)\Trigger.obj" \
	"$(INTDIR)\VBATest.obj" \
	"$(INTDIR)\VPinball.obj" \
	"$(INTDIR)\wall.obj" \
	"$(INTDIR)\wavread.obj" \
	"$(INTDIR)\VBATest.res"

".\VPinball.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

OutDir=.\ReleaseUMinDependency
TargetPath=.\VPinball.exe
InputPath=.\VPinball.exe
SOURCE="$(InputPath)"

"$(OUTDIR)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	if "%OS%"=="" goto NOTNT 
	if not "%OS%"=="Windows_NT" goto NOTNT 
	"$(TargetPath)" /RegServer 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
	echo Server registration done! 
	goto end 
	:NOTNT 
	echo Warning : Cannot register Unicode EXE on Windows 95 
	:end 
<< 
	

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("VBATest.dep")
!INCLUDE "VBATest.dep"
!ELSE 
!MESSAGE Warning: cannot find "VBATest.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "VBATest - Win32 Debug" || "$(CFG)" == "VBATest - Win32 Unicode Debug" || "$(CFG)" == "VBATest - Win32 Release MinSize" || "$(CFG)" == "VBATest - Win32 Release MinDependency" || "$(CFG)" == "VBATest - Win32 Unicode Release MinSize" || "$(CFG)" == "VBATest - Win32 Unicode Release MinDependency"
SOURCE=.\Pin\Ball.cpp

"$(INTDIR)\Ball.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\VBATest.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Bumper.cpp

"$(INTDIR)\Bumper.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\VBATest.pch"


SOURCE=.\Pin\collide.cpp

"$(INTDIR)\collide.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\VBATest.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Decal.cpp

"$(INTDIR)\Decal.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\VBATest.pch"


SOURCE=.\DragPoint.cpp

"$(INTDIR)\DragPoint.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\VBATest.pch"


SOURCE=.\Pin\draw.cpp

"$(INTDIR)\draw.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\VBATest.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\extern.cpp

"$(INTDIR)\extern.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\VBATest.pch"


SOURCE=.\Flipper.cpp

"$(INTDIR)\Flipper.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\VBATest.pch"


SOURCE=.\Gate.cpp

"$(INTDIR)\Gate.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\VBATest.pch"


SOURCE=.\Pin\hitflipper.cpp

"$(INTDIR)\hitflipper.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\VBATest.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Pin\HitPlunger.cpp

"$(INTDIR)\HitPlunger.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\VBATest.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\HitSur.cpp

"$(INTDIR)\HitSur.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\VBATest.pch"


SOURCE=.\IEditable.cpp

"$(INTDIR)\IEditable.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\VBATest.pch"


SOURCE=.\ISelect.cpp

"$(INTDIR)\ISelect.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\VBATest.pch"


SOURCE=.\Kicker.cpp

"$(INTDIR)\Kicker.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\VBATest.pch"


SOURCE=.\Light.cpp

"$(INTDIR)\Light.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\VBATest.pch"


SOURCE=.\Media\lzwreader.cpp

"$(INTDIR)\lzwreader.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\VBATest.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Media\lzwwriter.cpp

"$(INTDIR)\lzwwriter.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\VBATest.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Mesh.cpp

"$(INTDIR)\Mesh.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\VBATest.pch"


SOURCE=.\OLEAUTO.CPP

"$(INTDIR)\OLEAUTO.OBJ" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\VBATest.pch"


SOURCE=.\PaintSur.cpp

"$(INTDIR)\PaintSur.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\VBATest.pch"


SOURCE=.\Pin3D.cpp

"$(INTDIR)\Pin3D.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\VBATest.pch"


SOURCE=.\PinImage.cpp

"$(INTDIR)\PinImage.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\VBATest.pch"


SOURCE=.\PinSound.cpp

"$(INTDIR)\PinSound.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\VBATest.pch"


SOURCE=.\PinTable.cpp

"$(INTDIR)\PinTable.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\VBATest.pch"


SOURCE=.\PinUndo.cpp

"$(INTDIR)\PinUndo.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\VBATest.pch"


SOURCE=.\Pin\Player.cpp

"$(INTDIR)\Player.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\VBATest.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Plunger.cpp

"$(INTDIR)\Plunger.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\VBATest.pch"


SOURCE=.\Spinner.cpp

"$(INTDIR)\Spinner.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\VBATest.pch"


SOURCE=.\StdAfx.cpp

!IF  "$(CFG)" == "VBATest - Win32 Debug"

CPP_SWITCHES=/nologo /MLd /W3 /Gm /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /Fp"$(INTDIR)\VBATest.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\VBATest.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "VBATest - Win32 Unicode Debug"

CPP_SWITCHES=/nologo /MLd /W3 /Gm /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_UNICODE" /Fp"$(INTDIR)\VBATest.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\VBATest.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "VBATest - Win32 Release MinSize"

CPP_SWITCHES=/nologo /ML /W3 /O1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_ATL_DLL" /Fp"$(INTDIR)\VBATest.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\VBATest.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "VBATest - Win32 Release MinDependency"

CPP_SWITCHES=/nologo /ML /W3 /Zi /O1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_ATL_STATIC_REGISTRY" /Fp"$(INTDIR)\VBATest.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\VBATest.pch" : $(SOURCE) "$(INTDIR)" ".\VBATest.h"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "VBATest - Win32 Unicode Release MinSize"

CPP_SWITCHES=/nologo /ML /W3 /O1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_UNICODE" /D "_ATL_DLL" /D "_ATL_MIN_CRT" /Fp"$(INTDIR)\VBATest.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\VBATest.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "VBATest - Win32 Unicode Release MinDependency"

CPP_SWITCHES=/nologo /ML /W3 /O1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_UNICODE" /D "_ATL_STATIC_REGISTRY" /D "_ATL_MIN_CRT" /Fp"$(INTDIR)\VBATest.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\VBATest.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\Sur.cpp

"$(INTDIR)\Sur.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\VBATest.pch"


SOURCE=.\Surface.cpp

"$(INTDIR)\Surface.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\VBATest.pch"


SOURCE=.\Textbox.cpp

"$(INTDIR)\Textbox.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\VBATest.pch"


SOURCE=.\Timer.cpp

"$(INTDIR)\Timer.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\VBATest.pch"


SOURCE=.\Trigger.cpp

"$(INTDIR)\Trigger.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\VBATest.pch"


SOURCE=.\VBATest.cpp

"$(INTDIR)\VBATest.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\VBATest.pch"


SOURCE=.\VBATest.idl

!IF  "$(CFG)" == "VBATest - Win32 Debug"

MTL_SWITCHES=/tlb ".\VBATest.tlb" /h "VBATest.h" /iid "VBATest_i.c" /Oicf 

".\VBATest.tlb"	".\VBATest.h"	".\VBATest_i.c" : $(SOURCE) "$(INTDIR)"
	$(MTL) @<<
  $(MTL_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "VBATest - Win32 Unicode Debug"

MTL_SWITCHES=/tlb ".\VBATest.tlb" /h "VBATest.h" /iid "VBATest_i.c" /Oicf 

".\VBATest.tlb"	".\VBATest.h"	".\VBATest_i.c" : $(SOURCE) "$(INTDIR)"
	$(MTL) @<<
  $(MTL_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "VBATest - Win32 Release MinSize"

MTL_SWITCHES=/tlb ".\VBATest.tlb" /h "VBATest.h" /iid "VBATest_i.c" /Oicf 

".\VBATest.tlb"	".\VBATest.h"	".\VBATest_i.c" : $(SOURCE) "$(INTDIR)"
	$(MTL) @<<
  $(MTL_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "VBATest - Win32 Release MinDependency"

MTL_SWITCHES=/tlb ".\VBATest.tlb" /h "VBATest.h" /iid "VBATest_i.c" /Oicf 

".\VBATest.tlb"	".\VBATest.h"	".\VBATest_i.c" : $(SOURCE) "$(INTDIR)"
	$(MTL) @<<
  $(MTL_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "VBATest - Win32 Unicode Release MinSize"

MTL_SWITCHES=/tlb ".\VBATest.tlb" /h "VBATest.h" /iid "VBATest_i.c" /Oicf 

".\VBATest.tlb"	".\VBATest.h"	".\VBATest_i.c" : $(SOURCE) "$(INTDIR)"
	$(MTL) @<<
  $(MTL_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "VBATest - Win32 Unicode Release MinDependency"

MTL_SWITCHES=/tlb ".\VBATest.tlb" /h "VBATest.h" /iid "VBATest_i.c" /Oicf 

".\VBATest.tlb"	".\VBATest.h"	".\VBATest_i.c" : $(SOURCE) "$(INTDIR)"
	$(MTL) @<<
  $(MTL_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\VBATest.rc

"$(INTDIR)\VBATest.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=.\VPinball.cpp

"$(INTDIR)\VPinball.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\VBATest.pch"


SOURCE=.\Pin\wall.cpp

"$(INTDIR)\wall.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\VBATest.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Media\wavread.cpp

"$(INTDIR)\wavread.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\VBATest.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 

