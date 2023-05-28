@echo off

for /f "delims=" %%i in ('"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -property installationPath -format value') do call "%%i\VC\Auxiliary\Build\vcvars64.bat"
for /f "delims=" %%i in ('py -c "import sys, os; print(os.path.dirname(sys.executable))"') do @set PATH=%%i;%%i\Scripts;%PATH%
for /f "delims=" %%i in ('py -c "import os, sysconfig; print(sysconfig.get_path('scripts', f'{os.name}_user'))"') do @set PATH=%%i;%PATH%

git clone https://git.ffmpeg.org/ffmpeg.git %~dp0build\src\ffmpeg
cd %~dp0build\src\ffmpeg
git reset --hard
git checkout n5.1.2

chcp 65001

cd %~dp0
C:\msys64\usr\bin\bash.exe windows-build-ffmpeg.sh

pause