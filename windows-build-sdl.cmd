@echo off

for /f "delims=" %%i in ('"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -property installationPath -format value') do call "%%i\VC\Auxiliary\Build\vcvars64.bat"
for /f "delims=" %%i in ('py -c "import sys, os; print(os.path.dirname(sys.executable))"') do @set PATH=%%i;%%i\Scripts;%PATH%
for /f "delims=" %%i in ('py -c "import os, sysconfig; print(sysconfig.get_path('scripts', f'{os.name}_user'))"') do @set PATH=%%i;%PATH%

git clone https://github.com/libsdl-org/SDL.git %~dp0build\src\SDL
cd %~dp0build\src\SDL
git reset --hard
git checkout release-2.26.5

pip install cmake

mkdir %~dp0build\out\Debug\SDL
cd %~dp0build\out\Debug\SDL
cmake -G "NMake Makefiles" --install-prefix=%~dp0build -DBUILD_SHARED_LIBS=OFF -DCMAKE_BUILD_TYPE=Debug -B . %~dp0build\src\SDL
nmake
nmake install

mkdir %~dp0build\out\Release\SDL
cd %~dp0build\out\Release\SDL
cmake -G "NMake Makefiles" --install-prefix=%~dp0build -DBUILD_SHARED_LIBS=OFF -DCMAKE_BUILD_TYPE=Release -B . %~dp0build\src\SDL
nmake
nmake install

pause

