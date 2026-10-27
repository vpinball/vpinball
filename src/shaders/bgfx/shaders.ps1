# Regenerate the embedded BGFX shader headers (../bgfx_*.h) manually.
#
# The actual shader matrix and compile loop live in shaders.cmake, which is also
# what the CMake build (make/CMakeLists_shaders.txt) and the MSVC custom build
# step (make/vpx-core.vcxitems) run — edit that file, not this wrapper.
#
# Usage: ./shaders.ps1 [-Shaderc <path>] [-OutDir <dir>] [-Backends <list>] [-Group <name>]

param(
   [string]$Shaderc = "",
   [string]$OutDir = "",
   [string]$Backends = "mtl,essl,glsl,dxbc,dxil,spv",
   [string]$Group = ""
)

Set-Location $PSScriptRoot

if ($OutDir -eq "") { $OutDir = (Resolve-Path "..").Path }
$WorkDir = Join-Path $OutDir ".shader-frag"

if ($Shaderc -eq "") {
   foreach ($c in @(
      "..\..\..\third-party\tools\windows-x64\shaderc.exe",
      "..\..\..\third-party\tools\windows-x86\shaderc.exe",
      "..\..\..\third-party\tools\windows-mingw-x64\shaderc.exe",
      ".\shaderc.exe")) {
      if (Test-Path $c) { $Shaderc = (Resolve-Path $c).Path; break }
   }
}
if ($Shaderc -eq "") { Write-Error "shaderc not found: pass -Shaderc or run platforms/<platform>/external.sh"; exit 1 }

# shaderc loads the DXC compiler next to its own executable for s_6_0 (DXIL)
$env:PATH = "$(Split-Path $Shaderc);$env:PATH"

$groupArg = @()
if ($Group -ne "") { $groupArg = @("-DGROUP=$Group") }

cmake `
   "-DSHADERC=$Shaderc" `
   "-DOUT_DIR=$OutDir" `
   "-DWORK_DIR=$WorkDir" `
   "-DBACKENDS=$Backends" `
   @groupArg `
   -P shaders.cmake
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
