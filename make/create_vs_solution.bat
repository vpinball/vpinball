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

copy /V /Y "VisualPinball.net2022.sln" "../VisualPinball.sln"
copy /V /Y "VisualPinball.net2022.vcxproj" "../VisualPinball.vcxproj"
copy /V /Y "VisualPinball.net2022.vcxproj.filters" "../VisualPinball.vcxproj.filters"

if "%vsv%" == "2015" (
	cscript "simplereplace.wsf" //nologo /search:"<VCProjectVersion>17.0</VCProjectVersion>" /replace:"<VCProjectVersion>14.0</VCProjectVersion>" /in:"../VisualPinball.vcxproj" /out:"../VisualPinball.vcxproj"
	cscript "simplereplace.wsf" //nologo /search:"v143" /replace:"v140_xp" /in:"../VisualPinball.vcxproj" /out:"../VisualPinball.vcxproj"
	cscript "simplereplace.wsf" //nologo /search:"<WindowsTargetPlatformVersion>10.0</WindowsTargetPlatformVersion>" /replace:"" /in:"../VisualPinball.vcxproj" /out:"../VisualPinball.vcxproj"
    cscript "simplereplace.wsf" //nologo /search:"<LanguageStandard>stdcpp20</LanguageStandard>" /replace:"" /in:"../VisualPinball.vcxproj" /out:"../VisualPinball.vcxproj"
    cscript "simplereplace.wsf" //nologo /search:"/Zc:preprocessor " /replace:"" /in:"../VisualPinball.vcxproj" /out:"../VisualPinball.vcxproj"
	rem perhaps we should remove as well <AdditionalOptions>/Zc:__cplusplus %(AdditionalOptions)</AdditionalOptions>
)

if "%vsv%" == "2017" (
	cscript "simplereplace.wsf" //nologo /search:"<VCProjectVersion>17.0</VCProjectVersion>" /replace:"<VCProjectVersion>15.0</VCProjectVersion>" /in:"../VisualPinball.vcxproj" /out:"../VisualPinball.vcxproj"
	cscript "simplereplace.wsf" //nologo /search:"v143" /replace:"v141_xp" /in:"../VisualPinball.vcxproj" /out:"../VisualPinball.vcxproj"
	cscript "simplereplace.wsf" //nologo /search:"<WindowsTargetPlatformVersion>10.0</WindowsTargetPlatformVersion>" /replace:"" /in:"../VisualPinball.vcxproj" /out:"../VisualPinball.vcxproj"
    cscript "simplereplace.wsf" //nologo /search:"<LanguageStandard>stdcpp20</LanguageStandard>" /replace:"" /in:"../VisualPinball.vcxproj" /out:"../VisualPinball.vcxproj"
    cscript "simplereplace.wsf" //nologo /search:"/Zc:preprocessor " /replace:"" /in:"../VisualPinball.vcxproj" /out:"../VisualPinball.vcxproj"
	rem perhaps we should remove as well <AdditionalOptions>/Zc:__cplusplus %(AdditionalOptions)</AdditionalOptions>
)

if "%vsv%" == "2019" (
	cscript "simplereplace.wsf" //nologo /search:"v143" /replace:"v142" /in:"../VisualPinball.vcxproj" /out:"../VisualPinball.vcxproj"
)


REM ***************************************************************************************************
copy /V /Y "plugin-helloworld.net2022.vcxproj" "../plugin-helloworld.vcxproj"
copy /V /Y "plugin-helloworld.net2022.vcxproj.filters" "../plugin-helloworld.vcxproj.filters"

if "%vsv%" == "2015" (
	cscript "simplereplace.wsf" //nologo /search:"<VCProjectVersion>17.0</VCProjectVersion>" /replace:"<VCProjectVersion>14.0</VCProjectVersion>" /in:"../plugin-helloworld.vcxproj" /out:"../plugin-helloworld.vcxproj"
	cscript "simplereplace.wsf" //nologo /search:"v143" /replace:"v140_xp" /in:"../plugin-helloworld.vcxproj" /out:"../plugin-helloworld.vcxproj"
	cscript "simplereplace.wsf" //nologo /search:"<WindowsTargetPlatformVersion>10.0</WindowsTargetPlatformVersion>" /replace:"" /in:"../plugin-helloworld.vcxproj" /out:"../plugin-helloworld.vcxproj"
    cscript "simplereplace.wsf" //nologo /search:"<LanguageStandard>stdcpp20</LanguageStandard>" /replace:"" /in:"../plugin-helloworld.vcxproj" /out:"../plugin-helloworld.vcxproj"
	rem perhaps we should remove as well <AdditionalOptions>/Zc:__cplusplus %(AdditionalOptions)</AdditionalOptions>
)

if "%vsv%" == "2017" (
	cscript "simplereplace.wsf" //nologo /search:"<VCProjectVersion>17.0</VCProjectVersion>" /replace:"<VCProjectVersion>15.0</VCProjectVersion>" /in:"../plugin-helloworld.vcxproj" /out:"../plugin-helloworld.vcxproj"
	cscript "simplereplace.wsf" //nologo /search:"v143" /replace:"v141_xp" /in:"../plugin-helloworld.vcxproj" /out:"../plugin-helloworld.vcxproj"
	cscript "simplereplace.wsf" //nologo /search:"<WindowsTargetPlatformVersion>10.0</WindowsTargetPlatformVersion>" /replace:"" /in:"../plugin-helloworld.vcxproj" /out:"../plugin-helloworld.vcxproj"
    cscript "simplereplace.wsf" //nologo /search:"<LanguageStandard>stdcpp20</LanguageStandard>" /replace:"" /in:"../plugin-helloworld.vcxproj" /out:"../plugin-helloworld.vcxproj"
	rem perhaps we should remove as well <AdditionalOptions>/Zc:__cplusplus %(AdditionalOptions)</AdditionalOptions>
)

if "%vsv%" == "2019" (
	cscript "simplereplace.wsf" //nologo /search:"v143" /replace:"v142" /in:"../plugin-helloworld.vcxproj" /out:"../plugin-helloworld.vcxproj"
)


REM ***************************************************************************************************
copy /V /Y "plugin-helloscript.net2022.vcxproj" "../plugin-helloscript.vcxproj"
copy /V /Y "plugin-helloscript.net2022.vcxproj.filters" "../plugin-helloscript.vcxproj.filters"

if "%vsv%" == "2015" (
	cscript "simplereplace.wsf" //nologo /search:"<VCProjectVersion>17.0</VCProjectVersion>" /replace:"<VCProjectVersion>14.0</VCProjectVersion>" /in:"../plugin-helloscript.vcxproj" /out:"../plugin-helloscript.vcxproj"
	cscript "simplereplace.wsf" //nologo /search:"v143" /replace:"v140_xp" /in:"../plugin-helloscript.vcxproj" /out:"../plugin-helloscript.vcxproj"
	cscript "simplereplace.wsf" //nologo /search:"<WindowsTargetPlatformVersion>10.0</WindowsTargetPlatformVersion>" /replace:"" /in:"../plugin-helloscript.vcxproj" /out:"../plugin-helloscript.vcxproj"
    cscript "simplereplace.wsf" //nologo /search:"<LanguageStandard>stdcpp20</LanguageStandard>" /replace:"" /in:"../plugin-helloscript.vcxproj" /out:"../plugin-helloscript.vcxproj"
	rem perhaps we should remove as well <AdditionalOptions>/Zc:__cplusplus %(AdditionalOptions)</AdditionalOptions>
)

if "%vsv%" == "2017" (
	cscript "simplereplace.wsf" //nologo /search:"<VCProjectVersion>17.0</VCProjectVersion>" /replace:"<VCProjectVersion>15.0</VCProjectVersion>" /in:"../plugin-helloscript.vcxproj" /out:"../plugin-helloscript.vcxproj"
	cscript "simplereplace.wsf" //nologo /search:"v143" /replace:"v141_xp" /in:"../plugin-helloscript.vcxproj" /out:"../plugin-helloscript.vcxproj"
	cscript "simplereplace.wsf" //nologo /search:"<WindowsTargetPlatformVersion>10.0</WindowsTargetPlatformVersion>" /replace:"" /in:"../plugin-helloscript.vcxproj" /out:"../plugin-helloscript.vcxproj"
    cscript "simplereplace.wsf" //nologo /search:"<LanguageStandard>stdcpp20</LanguageStandard>" /replace:"" /in:"../plugin-helloscript.vcxproj" /out:"../plugin-helloscript.vcxproj"
	rem perhaps we should remove as well <AdditionalOptions>/Zc:__cplusplus %(AdditionalOptions)</AdditionalOptions>
)

if "%vsv%" == "2019" (
	cscript "simplereplace.wsf" //nologo /search:"v143" /replace:"v142" /in:"../plugin-helloscript.vcxproj" /out:"../plugin-helloscript.vcxproj"
)


REM ***************************************************************************************************
copy /V /Y "plugin-flexdmd.net2022.vcxproj" "../plugin-flexdmd.vcxproj"
copy /V /Y "plugin-flexdmd.net2022.vcxproj.filters" "../plugin-flexdmd.vcxproj.filters"

if "%vsv%" == "2015" (
	cscript "simplereplace.wsf" //nologo /search:"<VCProjectVersion>17.0</VCProjectVersion>" /replace:"<VCProjectVersion>14.0</VCProjectVersion>" /in:"../plugin-flexdmd.vcxproj" /out:"../plugin-flexdmd.vcxproj"
	cscript "simplereplace.wsf" //nologo /search:"v143" /replace:"v140_xp" /in:"../plugin-flexdmd.vcxproj" /out:"../plugin-flexdmd.vcxproj"
	cscript "simplereplace.wsf" //nologo /search:"<WindowsTargetPlatformVersion>10.0</WindowsTargetPlatformVersion>" /replace:"" /in:"../plugin-flexdmd.vcxproj" /out:"../plugin-flexdmd.vcxproj"
    cscript "simplereplace.wsf" //nologo /search:"<LanguageStandard>stdcpp20</LanguageStandard>" /replace:"" /in:"../plugin-flexdmd.vcxproj" /out:"../plugin-flexdmd.vcxproj"
	rem perhaps we should remove as well <AdditionalOptions>/Zc:__cplusplus %(AdditionalOptions)</AdditionalOptions>
)

if "%vsv%" == "2017" (
	cscript "simplereplace.wsf" //nologo /search:"<VCProjectVersion>17.0</VCProjectVersion>" /replace:"<VCProjectVersion>15.0</VCProjectVersion>" /in:"../plugin-flexdmd.vcxproj" /out:"../plugin-flexdmd.vcxproj"
	cscript "simplereplace.wsf" //nologo /search:"v143" /replace:"v141_xp" /in:"../plugin-flexdmd.vcxproj" /out:"../plugin-flexdmd.vcxproj"
	cscript "simplereplace.wsf" //nologo /search:"<WindowsTargetPlatformVersion>10.0</WindowsTargetPlatformVersion>" /replace:"" /in:"../plugin-flexdmd.vcxproj" /out:"../plugin-flexdmd.vcxproj"
    cscript "simplereplace.wsf" //nologo /search:"<LanguageStandard>stdcpp20</LanguageStandard>" /replace:"" /in:"../plugin-flexdmd.vcxproj" /out:"../plugin-flexdmd.vcxproj"
	rem perhaps we should remove as well <AdditionalOptions>/Zc:__cplusplus %(AdditionalOptions)</AdditionalOptions>
)

if "%vsv%" == "2019" (
	cscript "simplereplace.wsf" //nologo /search:"v143" /replace:"v142" /in:"../plugin-flexdmd.vcxproj" /out:"../plugin-flexdmd.vcxproj"
)


REM ***************************************************************************************************
copy /V /Y "plugin-alphadmd.net2022.vcxproj" "../plugin-alphadmd.vcxproj"
copy /V /Y "plugin-alphadmd.net2022.vcxproj.filters" "../plugin-alphadmd.vcxproj.filters"

if "%vsv%" == "2015" (
	cscript "simplereplace.wsf" //nologo /search:"<VCProjectVersion>17.0</VCProjectVersion>" /replace:"<VCProjectVersion>14.0</VCProjectVersion>" /in:"../plugin-alphadmd.vcxproj" /out:"../plugin-alphadmd.vcxproj"
	cscript "simplereplace.wsf" //nologo /search:"v143" /replace:"v140_xp" /in:"../plugin-alphadmd.vcxproj" /out:"../plugin-alphadmd.vcxproj"
	cscript "simplereplace.wsf" //nologo /search:"<WindowsTargetPlatformVersion>10.0</WindowsTargetPlatformVersion>" /replace:"" /in:"../plugin-alphadmd.vcxproj" /out:"../plugin-alphadmd.vcxproj"
    cscript "simplereplace.wsf" //nologo /search:"<LanguageStandard>stdcpp20</LanguageStandard>" /replace:"" /in:"../plugin-alphadmd.vcxproj" /out:"../plugin-alphadmd.vcxproj"
	rem perhaps we should remove as well <AdditionalOptions>/Zc:__cplusplus %(AdditionalOptions)</AdditionalOptions>
)

if "%vsv%" == "2017" (
	cscript "simplereplace.wsf" //nologo /search:"<VCProjectVersion>17.0</VCProjectVersion>" /replace:"<VCProjectVersion>15.0</VCProjectVersion>" /in:"../plugin-alphadmd.vcxproj" /out:"../plugin-alphadmd.vcxproj"
	cscript "simplereplace.wsf" //nologo /search:"v143" /replace:"v141_xp" /in:"../plugin-alphadmd.vcxproj" /out:"../plugin-alphadmd.vcxproj"
	cscript "simplereplace.wsf" //nologo /search:"<WindowsTargetPlatformVersion>10.0</WindowsTargetPlatformVersion>" /replace:"" /in:"../plugin-alphadmd.vcxproj" /out:"../plugin-alphadmd.vcxproj"
    cscript "simplereplace.wsf" //nologo /search:"<LanguageStandard>stdcpp20</LanguageStandard>" /replace:"" /in:"../plugin-alphadmd.vcxproj" /out:"../plugin-alphadmd.vcxproj"
	rem perhaps we should remove as well <AdditionalOptions>/Zc:__cplusplus %(AdditionalOptions)</AdditionalOptions>
)

if "%vsv%" == "2019" (
	cscript "simplereplace.wsf" //nologo /search:"v143" /replace:"v142" /in:"../plugin-alphadmd.vcxproj" /out:"../plugin-alphadmd.vcxproj"
)


REM ***************************************************************************************************
copy /V /Y "plugin-pinmame.net2022.vcxproj" "../plugin-pinmame.vcxproj"
copy /V /Y "plugin-pinmame.net2022.vcxproj.filters" "../plugin-pinmame.vcxproj.filters"

if "%vsv%" == "2015" (
	cscript "simplereplace.wsf" //nologo /search:"<VCProjectVersion>17.0</VCProjectVersion>" /replace:"<VCProjectVersion>14.0</VCProjectVersion>" /in:"../plugin-pinmame.vcxproj" /out:"../plugin-pinmame.vcxproj"
	cscript "simplereplace.wsf" //nologo /search:"v143" /replace:"v140_xp" /in:"../plugin-pinmame.vcxproj" /out:"../plugin-pinmame.vcxproj"
	cscript "simplereplace.wsf" //nologo /search:"<WindowsTargetPlatformVersion>10.0</WindowsTargetPlatformVersion>" /replace:"" /in:"../plugin-pinmame.vcxproj" /out:"../plugin-pinmame.vcxproj"
    cscript "simplereplace.wsf" //nologo /search:"<LanguageStandard>stdcpp20</LanguageStandard>" /replace:"" /in:"../plugin-pinmame.vcxproj" /out:"../plugin-pinmame.vcxproj"
	rem perhaps we should remove as well <AdditionalOptions>/Zc:__cplusplus %(AdditionalOptions)</AdditionalOptions>
)

if "%vsv%" == "2017" (
	cscript "simplereplace.wsf" //nologo /search:"<VCProjectVersion>17.0</VCProjectVersion>" /replace:"<VCProjectVersion>15.0</VCProjectVersion>" /in:"../plugin-pinmame.vcxproj" /out:"../plugin-pinmame.vcxproj"
	cscript "simplereplace.wsf" //nologo /search:"v143" /replace:"v141_xp" /in:"../plugin-pinmame.vcxproj" /out:"../plugin-pinmame.vcxproj"
	cscript "simplereplace.wsf" //nologo /search:"<WindowsTargetPlatformVersion>10.0</WindowsTargetPlatformVersion>" /replace:"" /in:"../plugin-pinmame.vcxproj" /out:"../plugin-pinmame.vcxproj"
    cscript "simplereplace.wsf" //nologo /search:"<LanguageStandard>stdcpp20</LanguageStandard>" /replace:"" /in:"../plugin-pinmame.vcxproj" /out:"../plugin-pinmame.vcxproj"
	rem perhaps we should remove as well <AdditionalOptions>/Zc:__cplusplus %(AdditionalOptions)</AdditionalOptions>
)

if "%vsv%" == "2019" (
	cscript "simplereplace.wsf" //nologo /search:"v143" /replace:"v142" /in:"../plugin-pinmame.vcxproj" /out:"../plugin-pinmame.vcxproj"
)


REM ***************************************************************************************************
copy /V /Y "plugin-serum.net2022.vcxproj" "../plugin-serum.vcxproj"
copy /V /Y "plugin-serum.net2022.vcxproj.filters" "../plugin-serum.vcxproj.filters"

if "%vsv%" == "2015" (
	cscript "simplereplace.wsf" //nologo /search:"<VCProjectVersion>17.0</VCProjectVersion>" /replace:"<VCProjectVersion>14.0</VCProjectVersion>" /in:"../plugin-serum.vcxproj" /out:"../plugin-serum.vcxproj"
	cscript "simplereplace.wsf" //nologo /search:"v143" /replace:"v140_xp" /in:"../plugin-serum.vcxproj" /out:"../plugin-serum.vcxproj"
	cscript "simplereplace.wsf" //nologo /search:"<WindowsTargetPlatformVersion>10.0</WindowsTargetPlatformVersion>" /replace:"" /in:"../plugin-serum.vcxproj" /out:"../plugin-serum.vcxproj"
    cscript "simplereplace.wsf" //nologo /search:"<LanguageStandard>stdcpp20</LanguageStandard>" /replace:"" /in:"../plugin-serum.vcxproj" /out:"../plugin-serum.vcxproj"
	rem perhaps we should remove as well <AdditionalOptions>/Zc:__cplusplus %(AdditionalOptions)</AdditionalOptions>
)

if "%vsv%" == "2017" (
	cscript "simplereplace.wsf" //nologo /search:"<VCProjectVersion>17.0</VCProjectVersion>" /replace:"<VCProjectVersion>15.0</VCProjectVersion>" /in:"../plugin-serum.vcxproj" /out:"../plugin-serum.vcxproj"
	cscript "simplereplace.wsf" //nologo /search:"v143" /replace:"v141_xp" /in:"../plugin-serum.vcxproj" /out:"../plugin-serum.vcxproj"
	cscript "simplereplace.wsf" //nologo /search:"<WindowsTargetPlatformVersion>10.0</WindowsTargetPlatformVersion>" /replace:"" /in:"../plugin-serum.vcxproj" /out:"../plugin-serum.vcxproj"
    cscript "simplereplace.wsf" //nologo /search:"<LanguageStandard>stdcpp20</LanguageStandard>" /replace:"" /in:"../plugin-serum.vcxproj" /out:"../plugin-serum.vcxproj"
	rem perhaps we should remove as well <AdditionalOptions>/Zc:__cplusplus %(AdditionalOptions)</AdditionalOptions>
)

if "%vsv%" == "2019" (
	cscript "simplereplace.wsf" //nologo /search:"v143" /replace:"v142" /in:"../plugin-serum.vcxproj" /out:"../plugin-serum.vcxproj"
)


REM ***************************************************************************************************
copy /V /Y "plugin-pinup-events.net2022.vcxproj" "../plugin-pinup-events.vcxproj"
copy /V /Y "plugin-pinup-events.net2022.vcxproj.filters" "../plugin-pinup-events.vcxproj.filters"

if "%vsv%" == "2015" (
	cscript "simplereplace.wsf" //nologo /search:"<VCProjectVersion>17.0</VCProjectVersion>" /replace:"<VCProjectVersion>14.0</VCProjectVersion>" /in:"../plugin-pinup-events.vcxproj" /out:"../plugin-pinup-events.vcxproj"
	cscript "simplereplace.wsf" //nologo /search:"v143" /replace:"v140_xp" /in:"../plugin-pinup-events.vcxproj" /out:"../plugin-pinup-events.vcxproj"
	cscript "simplereplace.wsf" //nologo /search:"<WindowsTargetPlatformVersion>10.0</WindowsTargetPlatformVersion>" /replace:"" /in:"../plugin-pinup-events.vcxproj" /out:"../plugin-pinup-events.vcxproj"
    cscript "simplereplace.wsf" //nologo /search:"<LanguageStandard>stdcpp20</LanguageStandard>" /replace:"" /in:"../plugin-pinup-events.vcxproj" /out:"../plugin-pinup-events.vcxproj"
	rem perhaps we should remove as well <AdditionalOptions>/Zc:__cplusplus %(AdditionalOptions)</AdditionalOptions>
)

if "%vsv%" == "2017" (
	cscript "simplereplace.wsf" //nologo /search:"<VCProjectVersion>17.0</VCProjectVersion>" /replace:"<VCProjectVersion>15.0</VCProjectVersion>" /in:"../plugin-pinup-events.vcxproj" /out:"../plugin-pinup-events.vcxproj"
	cscript "simplereplace.wsf" //nologo /search:"v143" /replace:"v141_xp" /in:"../plugin-pinup-events.vcxproj" /out:"../plugin-pinup-events.vcxproj"
	cscript "simplereplace.wsf" //nologo /search:"<WindowsTargetPlatformVersion>10.0</WindowsTargetPlatformVersion>" /replace:"" /in:"../plugin-pinup-events.vcxproj" /out:"../plugin-pinup-events.vcxproj"
    cscript "simplereplace.wsf" //nologo /search:"<LanguageStandard>stdcpp20</LanguageStandard>" /replace:"" /in:"../plugin-pinup-events.vcxproj" /out:"../plugin-pinup-events.vcxproj"
	rem perhaps we should remove as well <AdditionalOptions>/Zc:__cplusplus %(AdditionalOptions)</AdditionalOptions>
)

if "%vsv%" == "2019" (
	cscript "simplereplace.wsf" //nologo /search:"v143" /replace:"v142" /in:"../plugin-pinup-events.vcxproj" /out:"../plugin-pinup-events.vcxproj"
)

endlocal
