@echo off

SETLOCAL EnableDelayedExpansion

if not exist "%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" (
  echo "WARNING: You need VS 2017 version 15.2 or later (for vswhere.exe)"
)

for /f "usebackq tokens=*" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -version 17.0 -property installationPath`) do (
  set InstallDir=%%i
)

if exist "!InstallDir!\VC\Auxiliary\Build\vcvars64.bat" (
  call "!InstallDir!\VC\Auxiliary\Build\vcvars64.bat"
) else (
  echo "Could not find !InstallDir!\VC\Auxiliary\Build\vcvars64.bat"
)

mkdir vs2022_tools
cd vs2022_tools
cmake -G "Visual Studio 17 2022" -A x64 ..\ThirdParty\bgfx.cmake -DBGFX_BUILD_EXAMPLES=OFF -DBGFX_INSTALL=OFF 
devenv.com bgfx.sln /build Release /project shaderc
cd ..

cmake -G "Visual Studio 17 2022" -A x64 -B build_vs2022

pause