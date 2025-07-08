@echo off
goto :end_header
**************************************************************************
* This little batch script will create a Visual Studio solution & project
* for your favorite flavor of the environment between 2019 and 2022.
*
* Use: just run the script and enter the wanted environment. the solution
* will be generated at the root of the project.
**************************************************************************
:end_header
setlocal enabledelayedexpansion
set /p vsv=Which version would you like [2019/2022]?

if "%vsv%" == "2019" goto version_ok
if "%vsv%" == "2022" goto version_ok
echo Invalid version requested: %vsv%
endlocal
exit 1
:version_ok

md "../.build/vsproject"
copy /V /Y "VisualPinball.sln" "../.build/vsproject/VisualPinball.sln"
copy /V /Y "vpx.vcxproj" "../.build/vsproject/vpx.vcxproj"
copy /V /Y "vpx.vcxproj.filters" "../.build/vsproject/vpx.vcxproj.filters"
copy /V /Y "vpx-core.vcxitems" "../.build/vsproject/vpx-core.vcxitems"
copy /V /Y "vpx-core.vcxitems.filters" "../.build/vsproject/vpx-core.vcxitems.filters"
copy /V /Y "vpx-test.vcxproj" "../.build/vsproject/vpx-test.vcxproj"
copy /V /Y "vpx-test.vcxproj.filters" "../.build/vsproject/vpx-test.vcxproj.filters"

if "%vsv%" == "2019" (
	cscript "simplereplace.wsf" //nologo /search:"v143" /replace:"v142" /in:"../.build/vsproject/vpx.vcxproj" /out:"../.build/vsproject/vpx.vcxproj"
	cscript "simplereplace.wsf" //nologo /search:"v143" /replace:"v142" /in:"../.build/vsproject/vpx-core.vcxproj" /out:"../.build/vsproject/vpx-core.vcxproj"
	cscript "simplereplace.wsf" //nologo /search:"v143" /replace:"v142" /in:"../.build/vsproject/vpx-test.vcxproj" /out:"../.build/vsproject/vpx-test.vcxproj"
)


REM ***************************************************************************************************
copy /V /Y "plugin-alphadmd.vcxproj" "../.build/vsproject/plugin-alphadmd.vcxproj"
copy /V /Y "plugin-alphadmd.vcxproj.filters" "../.build/vsproject/plugin-alphadmd.vcxproj.filters"

if "%vsv%" == "2019" (
	cscript "simplereplace.wsf" //nologo /search:"v143" /replace:"v142" /in:"../.build/vsproject/plugin-alphadmd.vcxproj" /out:"../.build/vsproject/plugin-alphadmd.vcxproj"
)


REM ***************************************************************************************************
copy /V /Y "plugin-b2s.vcxproj" "../.build/vsproject/plugin-b2s.vcxproj"
copy /V /Y "plugin-b2s.vcxproj.filters" "../.build/vsproject/plugin-b2s.vcxproj.filters"

if "%vsv%" == "2019" (
	cscript "simplereplace.wsf" //nologo /search:"v143" /replace:"v142" /in:"../.build/vsproject/plugin-b2s.vcxproj" /out:"../.build/vsproject/plugin-b2s.vcxproj"
)


REM ***************************************************************************************************
copy /V /Y "plugin-dmdutil.vcxproj" "../.build/vsproject/plugin-dmdutil.vcxproj"
copy /V /Y "plugin-dmdutil.vcxproj.filters" "../.build/vsproject/plugin-dmdutil.vcxproj.filters"

if "%vsv%" == "2019" (
	cscript "simplereplace.wsf" //nologo /search:"v143" /replace:"v142" /in:"../.build/vsproject/plugin-dmdutil.vcxproj" /out:"../.build/vsproject/plugin-dmdutil.vcxproj"
)


REM ***************************************************************************************************
copy /V /Y "plugin-dof.vcxproj" "../.build/vsproject/plugin-dof.vcxproj"
copy /V /Y "plugin-dof.vcxproj.filters" "../.build/vsproject/plugin-dof.vcxproj.filters"

if "%vsv%" == "2019" (
	cscript "simplereplace.wsf" //nologo /search:"v143" /replace:"v142" /in:"../.build/vsproject/plugin-dof.vcxproj" /out:"../.build/vsproject/plugin-dof.vcxproj"
)


REM ***************************************************************************************************
copy /V /Y "plugin-flexdmd.vcxproj" "../.build/vsproject/plugin-flexdmd.vcxproj"
copy /V /Y "plugin-flexdmd.vcxproj.filters" "../.build/vsproject/plugin-flexdmd.vcxproj.filters"

if "%vsv%" == "2019" (
	cscript "simplereplace.wsf" //nologo /search:"v143" /replace:"v142" /in:"../.build/vsproject/plugin-flexdmd.vcxproj" /out:"../.build/vsproject/plugin-flexdmd.vcxproj"
)


REM ***************************************************************************************************
copy /V /Y "plugin-helloworld.vcxproj" "../.build/vsproject/plugin-helloworld.vcxproj"
copy /V /Y "plugin-helloworld.vcxproj.filters" "../.build/vsproject/plugin-helloworld.vcxproj.filters"

if "%vsv%" == "2019" (
	cscript "simplereplace.wsf" //nologo /search:"v143" /replace:"v142" /in:"../.build/vsproject/plugin-helloworld.vcxproj" /out:"../.build/vsproject/plugin-helloworld.vcxproj"
)


REM ***************************************************************************************************
copy /V /Y "plugin-helloscript.vcxproj" "../.build/vsproject/plugin-helloscript.vcxproj"
copy /V /Y "plugin-helloscript.vcxproj.filters" "../.build/vsproject/plugin-helloscript.vcxproj.filters"

if "%vsv%" == "2019" (
	cscript "simplereplace.wsf" //nologo /search:"v143" /replace:"v142" /in:"../.build/vsproject/plugin-helloscript.vcxproj" /out:"../.build/vsproject/plugin-helloscript.vcxproj"
)


REM ***************************************************************************************************
copy /V /Y "plugin-pinmame.vcxproj" "../.build/vsproject/plugin-pinmame.vcxproj"
copy /V /Y "plugin-pinmame.vcxproj.filters" "../.build/vsproject/plugin-pinmame.vcxproj.filters"

if "%vsv%" == "2019" (
	cscript "simplereplace.wsf" //nologo /search:"v143" /replace:"v142" /in:"../.build/vsproject/plugin-pinmame.vcxproj" /out:"../.build/vsproject/plugin-pinmame.vcxproj"
)


REM ***************************************************************************************************
copy /V /Y "plugin-pinup-events.vcxproj" "../.build/vsproject/plugin-pinup-events.vcxproj"
copy /V /Y "plugin-pinup-events.vcxproj.filters" "../.build/vsproject/plugin-pinup-events.vcxproj.filters"

if "%vsv%" == "2019" (
	cscript "simplereplace.wsf" //nologo /search:"v143" /replace:"v142" /in:"../.build/vsproject/plugin-pinup-events.vcxproj" /out:"../.build/vsproject/plugin-pinup-events.vcxproj"
)


REM ***************************************************************************************************
copy /V /Y "plugin-pup.vcxproj" "../.build/vsproject/plugin-pup.vcxproj"
copy /V /Y "plugin-pup.vcxproj.filters" "../.build/vsproject/plugin-pup.vcxproj.filters"

if "%vsv%" == "2019" (
	cscript "simplereplace.wsf" //nologo /search:"v143" /replace:"v142" /in:"../.build/vsproject/plugin-pup.vcxproj" /out:"../.build/vsproject/plugin-pup.vcxproj"
)


REM ***************************************************************************************************
copy /V /Y "plugin-remote-control.vcxproj" "../.build/vsproject/plugin-remote-control.vcxproj"
copy /V /Y "plugin-remote-control.vcxproj.filters" "../.build/vsproject/plugin-remote-control.vcxproj.filters"

if "%vsv%" == "2019" (
	cscript "simplereplace.wsf" //nologo /search:"v143" /replace:"v142" /in:"../.build/vsproject/plugin-remote-control.vcxproj" /out:"../.build/vsproject/plugin-remote-control.vcxproj"
)


REM ***************************************************************************************************
copy /V /Y "plugin-scoreview.vcxproj" "../.build/vsproject/plugin-scoreview.vcxproj"
copy /V /Y "plugin-scoreview.vcxproj.filters" "../.build/vsproject/plugin-scoreview.vcxproj.filters"

if "%vsv%" == "2019" (
	cscript "simplereplace.wsf" //nologo /search:"v143" /replace:"v142" /in:"../.build/vsproject/plugin-scoreview.vcxproj" /out:"../.build/vsproject/plugin-b2s.vcxproj"
)


REM ***************************************************************************************************
copy /V /Y "plugin-serum.vcxproj" "../.build/vsproject/plugin-serum.vcxproj"
copy /V /Y "plugin-serum.vcxproj.filters" "../.build/vsproject/plugin-serum.vcxproj.filters"

if "%vsv%" == "2019" (
	cscript "simplereplace.wsf" //nologo /search:"v143" /replace:"v142" /in:"../.build/vsproject/plugin-serum.vcxproj" /out:"../.build/vsproject/plugin-serum.vcxproj"
)


endlocal
