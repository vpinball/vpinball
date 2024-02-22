@echo off
goto :end_header
**************************************************************************
* This little batch script will create a Visual Studio solution & project
* for your favorite flavor of the environment between 2015 and 2022.
*
* Use: just run the script and enter the wanted environment. the solution
* will be generated at the root of the project.
**************************************************************************
:end_header
setlocal enabledelayedexpansion
set /p vsv=Which version would you like [2015/2017/2019/2022]?

if "%vsv%" == "2015" goto version_ok
if "%vsv%" == "2017" goto version_ok
if "%vsv%" == "2019" goto version_ok
if "%vsv%" == "2022" goto version_ok
echo Invalid version requested: %vsv%
endlocal
exit 1
:version_ok

copy /V /-Y "VisualPinball.net2022.sln" "../VisualPinball.sln"
copy /V /-Y "VisualPinball.net2022.vcxproj" "../VisualPinball.vcxproj"
copy /V /-Y "VisualPinball.net2022.vcxproj.filters" "../VisualPinball.vcxproj.filters"

if "%vsv%" == "2015" (
	cscript "simplereplace.wsf" //nologo /search:"XXToolVersionXX" /replace:"14.0" /in:"../VisualPinball.vcxproj" /out:"../VisualPinball.vcxproj"
	cscript "simplereplace.wsf" //nologo /search:"<VCProjectVersion>16.0</VCProjectVersion>" /replace:"<VCProjectVersion>14.0</VCProjectVersion>" /in:"../VisualPinball.vcxproj" /out:"../VisualPinball.vcxproj"
	cscript "simplereplace.wsf" //nologo /search:"v143" /replace:"v140_xp" /in:"../VisualPinball.vcxproj" /out:"../VisualPinball.vcxproj"
	cscript "simplereplace.wsf" //nologo /search:"<WindowsTargetPlatformVersion>10.0</WindowsTargetPlatformVersion>" /replace:"" /in:"../VisualPinball.vcxproj" /out:"../VisualPinball.vcxproj"
    cscript "simplereplace.wsf" //nologo /search:"<LanguageStandard>stdcpp20</LanguageStandard>" /replace:"" /in:"../VisualPinball.vcxproj" /out:"../VisualPinball.vcxproj"
	rem perhaps we should remove as well <AdditionalOptions>/Zc:__cplusplus %(AdditionalOptions)</AdditionalOptions>
)

if "%vsv%" == "2017" (
	cscript "simplereplace.wsf" //nologo /search:"XXToolVersionXX" /replace:"15.0" /in:"../VisualPinball.vcxproj" /out:"../VisualPinball.vcxproj"
	cscript "simplereplace.wsf" //nologo /search:"<VCProjectVersion>16.0</VCProjectVersion>" /replace:"<VCProjectVersion>15.0</VCProjectVersion>" /in:"../VisualPinball.vcxproj" /out:"../VisualPinball.vcxproj"
	cscript "simplereplace.wsf" //nologo /search:"v143" /replace:"v141_xp" /in:"../VisualPinball.vcxproj" /out:"../VisualPinball.vcxproj"
	cscript "simplereplace.wsf" //nologo /search:"<WindowsTargetPlatformVersion>10.0</WindowsTargetPlatformVersion>" /replace:"" /in:"../VisualPinball.vcxproj" /out:"../VisualPinball.vcxproj"
    cscript "simplereplace.wsf" //nologo /search:"<LanguageStandard>stdcpp20</LanguageStandard>" /replace:"" /in:"../VisualPinball.vcxproj" /out:"../VisualPinball.vcxproj"
	rem perhaps we should remove as well <AdditionalOptions>/Zc:__cplusplus %(AdditionalOptions)</AdditionalOptions>
)

if "%vsv%" == "2019" (
	cscript "simplereplace.wsf" //nologo /search:"XXToolVersionXX" /replace:"16.0" /in:"../VisualPinball.vcxproj" /out:"../VisualPinball.vcxproj"
	cscript "simplereplace.wsf" //nologo /search:"v143" /replace:"v142" /in:"../VisualPinball.vcxproj" /out:"../VisualPinball.vcxproj"
)

if "%vsv%" == "2022" (
	cscript "simplereplace.wsf" //nologo /search:"XXToolVersionXX" /replace:"16.0" /in:"../VisualPinball.vcxproj" /out:"../VisualPinball.vcxproj"
)

endlocal
