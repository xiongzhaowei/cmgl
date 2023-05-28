@echo off

for /f "delims=" %%i in ('"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -property installationPath -format value') do call "%%i\VC\Auxiliary\Build\vcvars64.bat"
for /f "delims=" %%i in ('py -c "import sys, os; print(os.path.dirname(sys.executable))"') do @set PATH=%%i;%%i\Scripts;%PATH%
for /f "delims=" %%i in ('py -c "import os, sysconfig; print(sysconfig.get_path('scripts', f'{os.name}_user'))"') do @set PATH=%%i;%PATH%

pip install cmake

mkdir %~dp0build\project
cd %~dp0build\project

cmake -B . %~dp0

pause
