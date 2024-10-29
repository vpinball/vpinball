@echo off
setlocal enabledelayedexpansion

:header
goto :end_header
**************************************************************************
* Win_Build Script
* Generates/Builds Windows-based project files from CMake workflow scripts
* v1.0
* Dave Roscoe
* 08/24/2023
*-------------------------------------------------------------------------
* SCRIPT NOTES
* - Creates individual solution and project files for each
*   application and library, as selected.  It cannot currently create a
*   single solution file for all projects
* - Utilizes the CMake scripts used for the automated workflows.  It
*   always generates projects that are current, and working, provided the
*   workflows are working and local configuration is correct
* - If you switch to/from Win32/x64 builds after building a project,
*   you must manually delete the CMakeCache.txt file and CMakeFiles
*   directory manually before running the script
*-------------------------------------------------------------------------
* FUTURE WORK
* - Re-implement in Powershell
* - Add single solution file generation for all projects
* - Add ability to clean up targets and CMake artifacts.
*-------------------------------------------------------------------------
:end_header

@REM provides more intuitive behavior for EXIT cmd
If "%selfWrapped%"=="" (
  REM Allow "exit" cmd to terminate the batch file, and all subroutines
  REM without terminating the shell process
  set selfWrapped=1
  %ComSpec% /s /c ""%~0" %*"
  goto :EOF
)

@REM Define and set default values.
set DEF_PRJ=ALL
set DEF_SAM=YES
set DEF_PLA=WIN32
set DEF_GEN=9
set DEF_BLD=YES
set DEF_CFG=DEBUG

@REM Declare internal variables
set PROJECT=
set PLATFORM=
set GENERATOR=
set BUILD=
set CONFIG=

set GEN_VPINBALL=""
set GEN_VPINBALL_GL=""

set SCRIPT_PLA=""

@REM CMake variables
set OPTIONS=
set TOOLS=-T v141_xp

@REM Print help (terminates script)
if "%~1" == "/?" (goto :print_help)

@REM Parse command line
call :parse_commands %*
if %errorlevel% neq 0 goto :failed

@REM Process default parameters
call :process_defaults
if %errorlevel% neq 0 goto :failed

:main_proc
@REM Display active parameters for this build
call :print_globals

@REM *************************************************************************
@REM BUILD VPINBALL
@REM *************************************************************************
if %GEN_VPINBALL% == YES (
   set OUTPATH=build/vpinball/%PLATFORM%/

   copy /Y CMakeLists_%SCRIPT_PLA%.txt CMakeLists.txt
   cmake %TOOLS% -G %GENERATOR% -A %PLATFORM% -B !OUTPATH!

    if %BUILD%==YES (
        cmake --build !OUTPATH! --config %CONFIG%
    )
)
if %errorlevel% neq 0 (goto :failed)

@REM *************************************************************************
@REM BUILD VPINBALL_GL
@REM *************************************************************************
if %GEN_VPINBALL_GL% == YES (
   set OUTPATH=build/vpinball_gl/%PLATFORM%/
    
   copy /Y CMakeLists_gl-%SCRIPT_PLA%.txt CMakeLists.txt
   cmake -G %GENERATOR% -A %PLATFORM% -B !OUTPATH!

    if %BUILD% == YES (
        cmake --build !OUTPATH! --config %CONFIG%
    )
)
if %errorlevel% neq 0 (goto :failed)

@REM *************************************************************************
@REM END OF SCRIPT
@REM *************************************************************************

echo --- BUILD SUCCEEDED ---

endlocal
exit /b %errorlevel%


@REM *************************************************************************
@REM SUBROUTINES
@REM *************************************************************************

:process_defaults
@REM Set default values for any unprovided commandline arguments
    if not defined PROJECT (call :parse_project %DEF_PRJ%)
    if %errorlevel% neq 0 (exit /b 1)

    if not defined PLATFORM call :parse_platform %DEF_PLA%
    if %errorlevel% neq 0 (exit /b 1)

    if not defined GENERATOR call :parse_generator %DEF_GEN%
    if %errorlevel% neq 0 (exit /b 1)

    if not defined BUILD call :parse_build %DEF_BLD%
    if %errorlevel% neq 0 (exit /b 1)

    if not defined CONFIG call :parse_buildtype %DEF_CFG%
    if %errorlevel% neq 0 (exit /b 1)
    
    exit /b


:parse_commands
:cmd_loop
    @REM Check for more arguments
    if "%1" == "" (exit /b)

    @REM Tokenize argument:param pair
    for /f "tokens=1,2 delims=:" %%a in ("%1") do (
        @REM If this is done inline, it will introduce a space after
        @REM %%a which will be added to ARG (undesirable)
        set ARG=%%a
        set VAL=%%b
    )

    if "%VAL%" == "" (
        echo *** ERROR: NO PARAMETER FOR "%ARG%" PROVIDED ***
        exit /b 1
    )

    if /I %ARG% == PRJ (call :parse_project %VAL%
    ) else if /I %ARG% == PLA (
        call :parse_platform %VAL%
    ) else if /I %ARG% == GEN (
        call :parse_generator %VAL%
    ) else if /I %ARG% == BLD (
        call :parse_build %VAL%
    ) else if /I %ARG% == CFG (
        call :parse_buildtype %VAL%
    ) else (
        echo *** ERROR: UNKNOWN ARGUMENT: "%ARG%" ***
        exit /b 1
    )

    @REM  continue parsing if no error
    if %errorlevel% equ 0 (
        shift
        goto :cmd_loop
    )
    
    @REM Error ocurred
    exit /b 1


:parse_project
    if /I %1 == ALL (
        set GEN_VPINBALL=YES
        set GEN_VPINBALL_GL=YES
    ) else if /I %1 == vpinball (
        set GEN_VPINBALL=YES
    ) else if /I %1 == vpinballgl (
        set GEN_VPINBALL_GL=YES
    ) else (
        echo *** ERROR: INVALID "PRJ" PARAMETER: "%1" ***
        exit /b 1
    )

    set PROJECT=%1
    exit /b


:parse_platform
    if /I %1 == Win32 (
        set PLATFORM=Win32
        set SCRIPT_PLA=win-x86
    ) else if /I %1 == x64 (
        set PLATFORM=x64
        set SCRIPT_PLA=win-x64
    ) else (
        echo *** ERROR: INVALID "PLA" PARAMETER: "%1" ***
        exit /b 1
    )
    exit /b


:parse_generator
    @REM NOTE: The fact a generator is listed here is not an indication that the
    @REM       build using any given generator is known to work.  This is simply 
    @REM       a list of all Windows generators CMake supports for convenience
    @REM       of reference on the command line
    @REM
    @REM See: https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html
    set GEN[0]="Visual Studio 6"
    set GEN[1]="Visual Studio 7"
    set GEN[2]="Visual Studio 7 .NET 2003"
    set GEN[3]="Visual Studio 8 2005"
    set GEN[4]="Visual Studio 9 2008
    set GEN[5]="Visual Studio 10 2010"
    set GEN[6]="Visual Studio 11 2012"
    set GEN[7]="Visual Studio 12 2013"
    set GEN[8]="Visual Studio 14 2015"
    set GEN[9]="Visual Studio 15 2017"
    set GEN[10]="Visual Studio 16 2019"
    set GEN[11]="Visual Studio 17 2022"

    if defined GEN[%1] (
        set GENERATOR=!GEN[%1]!
    ) else (
        echo *** ERROR: INVALID "GEN" PARAMETER: "%1" ***
        exit /b 1
    )

    exit /b


:parse_build
    set BUILD_PROJECT=
    
    @REM Simulate logical OR
    if /I %1 == Y set BUILD_PROJECT=1
    if /I %1 == YES set BUILD_PROJECT=1
        
    if /I %1 == N set BUILD_PROJECT=0
    if /I %1 == NO set BUILD_PROJECT=0
    
    if not defined BUILD_PROJECT (
        echo *** INVALID "BLD" PARAMETER: "%1" ***
        exit /b 1
    )
    
    if %BUILD_PROJECT% == 1 (
        set BUILD=YES
    ) else (
        set BUILD=NO
    )
    exit /b


:parse_buildtype
    set BUILD_CFG=
    
    @REM Simulate logical OR
    if /I %1 == D set BUILD_CFG=1
    if /I %1 == DEBUG set BUILD_CFG=1
    
    if /I %1 == R set BUILD_CFG=0
    if /I %1 == RELEASE set BUILD_CFG=0

    if not defined BUILD_CFG (
        echo *** INVALID "CFG" PARAMETER: "%1" ***
        exit /b 1
    )

    if %BUILD_CFG% == 1 (
        set CONFIG=DEBUG
    ) else (
        set CONFIG=RELEASE
    )
    exit /b


:print_globals
    echo --- GLOBAL VARIABLES ---
    echo PROJECT         : %PROJECT%
    echo PLATFORM        : %PLATFORM%
    echo GENERATOR       : %GENERATOR%
    echo BUILD           : %BUILD%
    echo CONFIG          : %CONFIG%
    echo GEN_VPINBALL    : %GEN_VPINBALL%
    echo GEN_VPINBALL_GL : %GEN_VPINBALL_GL%

    exit /b


:print_help
    more win_build_help.txt
    exit 0


:failed
    echo *** ERROR: ABNORMAL SCRIPT TERMINATION ***
    exit %errorlevel%
