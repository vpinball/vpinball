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
copy /V /Y "VisualPinball.net2022.sln" "../.build/vsproject/VisualPinball.sln"
copy /V /Y "VisualPinball.net2022.vcxproj" "../.build/vsproject/VisualPinball.vcxproj"
copy /V /Y "VisualPinball.net2022.vcxproj.filters" "../.build/vsproject/VisualPinball.vcxproj.filters"

if "%vsv%" == "2019" (
	cscript "simplereplace.wsf" //nologo /search:"v143" /replace:"v142" /in:"../.build/vsproject/VisualPinball.vcxproj" /out:"../.build/vsproject/VisualPinball.vcxproj"
)


REM ***************************************************************************************************
copy /V /Y "plugin-helloworld.net2022.vcxproj" "../.build/vsproject/plugin-helloworld.vcxproj"
copy /V /Y "plugin-helloworld.net2022.vcxproj.filters" "../.build/vsproject/plugin-helloworld.vcxproj.filters"

if "%vsv%" == "2019" (
	cscript "simplereplace.wsf" //nologo /search:"v143" /replace:"v142" /in:"../.build/vsproject/plugin-helloworld.vcxproj" /out:"../.build/vsproject/plugin-helloworld.vcxproj"
)


REM ***************************************************************************************************
copy /V /Y "plugin-helloscript.net2022.vcxproj" "../.build/vsproject/plugin-helloscript.vcxproj"
copy /V /Y "plugin-helloscript.net2022.vcxproj.filters" "../.build/vsproject/plugin-helloscript.vcxproj.filters"

if "%vsv%" == "2019" (
	cscript "simplereplace.wsf" //nologo /search:"v143" /replace:"v142" /in:"../.build/vsproject/plugin-helloscript.vcxproj" /out:"../.build/vsproject/plugin-helloscript.vcxproj"
)


REM ***************************************************************************************************
copy /V /Y "plugin-flexdmd.net2022.vcxproj" "../.build/vsproject/plugin-flexdmd.vcxproj"
copy /V /Y "plugin-flexdmd.net2022.vcxproj.filters" "../.build/vsproject/plugin-flexdmd.vcxproj.filters"

if "%vsv%" == "2019" (
	cscript "simplereplace.wsf" //nologo /search:"v143" /replace:"v142" /in:"../.build/vsproject/plugin-flexdmd.vcxproj" /out:"../.build/vsproject/plugin-flexdmd.vcxproj"
)


REM ***************************************************************************************************
copy /V /Y "plugin-alphadmd.net2022.vcxproj" "../.build/vsproject/plugin-alphadmd.vcxproj"
copy /V /Y "plugin-alphadmd.net2022.vcxproj.filters" "../.build/vsproject/plugin-alphadmd.vcxproj.filters"

if "%vsv%" == "2019" (
	cscript "simplereplace.wsf" //nologo /search:"v143" /replace:"v142" /in:"../.build/vsproject/plugin-alphadmd.vcxproj" /out:"../.build/vsproject/plugin-alphadmd.vcxproj"
)


REM ***************************************************************************************************
copy /V /Y "plugin-scoreview.net2022.vcxproj" "../.build/vsproject/plugin-scoreview.vcxproj"
copy /V /Y "plugin-scoreview.net2022.vcxproj.filters" "../.build/vsproject/plugin-scoreview.vcxproj.filters"

if "%vsv%" == "2019" (
	cscript "simplereplace.wsf" //nologo /search:"v143" /replace:"v142" /in:"../.build/vsproject/plugin-scoreview.vcxproj" /out:"../.build/vsproject/plugin-b2s.vcxproj"
)


REM ***************************************************************************************************
copy /V /Y "plugin-b2s.net2022.vcxproj" "../.build/vsproject/plugin-b2s.vcxproj"
copy /V /Y "plugin-b2s.net2022.vcxproj.filters" "../.build/vsproject/plugin-b2s.vcxproj.filters"

if "%vsv%" == "2019" (
	cscript "simplereplace.wsf" //nologo /search:"v143" /replace:"v142" /in:"../.build/vsproject/plugin-b2s.vcxproj" /out:"../.build/vsproject/plugin-b2s.vcxproj"
)


REM ***************************************************************************************************
copy /V /Y "plugin-pup.net2022.vcxproj" "../.build/vsproject/plugin-pup.vcxproj"
copy /V /Y "plugin-pup.net2022.vcxproj.filters" "../.build/vsproject/plugin-pup.vcxproj.filters"

if "%vsv%" == "2019" (
	cscript "simplereplace.wsf" //nologo /search:"v143" /replace:"v142" /in:"../.build/vsproject/plugin-pup.vcxproj" /out:"../.build/vsproject/plugin-pup.vcxproj"
)


REM ***************************************************************************************************
copy /V /Y "plugin-pinmame.net2022.vcxproj" "../.build/vsproject/plugin-pinmame.vcxproj"
copy /V /Y "plugin-pinmame.net2022.vcxproj.filters" "../.build/vsproject/plugin-pinmame.vcxproj.filters"

if "%vsv%" == "2019" (
	cscript "simplereplace.wsf" //nologo /search:"v143" /replace:"v142" /in:"../.build/vsproject/plugin-pinmame.vcxproj" /out:"../.build/vsproject/plugin-pinmame.vcxproj"
)


REM ***************************************************************************************************
copy /V /Y "plugin-remote-control.net2022.vcxproj" "../.build/vsproject/plugin-remote-control.vcxproj"
copy /V /Y "plugin-remote-control.net2022.vcxproj.filters" "../.build/vsproject/plugin-remote-control.vcxproj.filters"

if "%vsv%" == "2019" (
	cscript "simplereplace.wsf" //nologo /search:"v143" /replace:"v142" /in:"../.build/vsproject/plugin-remote-control.vcxproj" /out:"../.build/vsproject/plugin-remote-control.vcxproj"
)


REM ***************************************************************************************************
copy /V /Y "plugin-serum.net2022.vcxproj" "../.build/vsproject/plugin-serum.vcxproj"
copy /V /Y "plugin-serum.net2022.vcxproj.filters" "../.build/vsproject/plugin-serum.vcxproj.filters"

if "%vsv%" == "2019" (
	cscript "simplereplace.wsf" //nologo /search:"v143" /replace:"v142" /in:"../.build/vsproject/plugin-serum.vcxproj" /out:"../.build/vsproject/plugin-serum.vcxproj"
)


REM ***************************************************************************************************
copy /V /Y "plugin-pinup-events.net2022.vcxproj" "../.build/vsproject/plugin-pinup-events.vcxproj"
copy /V /Y "plugin-pinup-events.net2022.vcxproj.filters" "../.build/vsproject/plugin-pinup-events.vcxproj.filters"

if "%vsv%" == "2019" (
	cscript "simplereplace.wsf" //nologo /search:"v143" /replace:"v142" /in:"../.build/vsproject/plugin-pinup-events.vcxproj" /out:"../.build/vsproject/plugin-pinup-events.vcxproj"
)


REM ***************************************************************************************************
copy /V /Y "plugin-dmdutil.net2022.vcxproj" "../.build/vsproject/plugin-dmdutil.vcxproj"
copy /V /Y "plugin-dmdutil.net2022.vcxproj.filters" "../.build/vsproject/plugin-dmdutil.vcxproj.filters"

if "%vsv%" == "2019" (
	cscript "simplereplace.wsf" //nologo /search:"v143" /replace:"v142" /in:"../.build/vsproject/plugin-dmdutil.vcxproj" /out:"../.build/vsproject/plugin-dmdutil.vcxproj"
)


endlocal
