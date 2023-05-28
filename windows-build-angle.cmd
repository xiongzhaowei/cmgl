@echo off

for /f "delims=" %%i in ('"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -property installationPath -format value') do call "%%i\VC\Auxiliary\Build\vcvars64.bat"
for /f "delims=" %%i in ('py -c "import sys, os; print(os.path.dirname(sys.executable))"') do @set PATH=%%i;%%i\Scripts;%PATH%
for /f "delims=" %%i in ('py -c "import os, sysconfig; print(sysconfig.get_path('scripts', f'{os.name}_user'))"') do @set PATH=%%i;%PATH%

mkdir %~dp0build\src\angle

cd %~dp0build
git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
set PATH=%~dp0build\depot_tools;%PATH%

cd %~dp0build\src\angle
set DEPOT_TOOLS_WIN_TOOLCHAIN=0
call fetch angle
call git checkout chromium/5776
call gclient sync || exit /b 1
call gn gen out/Release --args="angle_build_all=false is_debug=false angle_has_frame_capture=false angle_enable_gl=false angle_enable_vulkan=false angle_enable_d3d9=false angle_enable_null=false"
autoninja -C out/Release libEGL libGLESv2 libGLESv1_CM

