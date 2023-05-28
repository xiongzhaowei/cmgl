export PATH=/usr/bin:$PATH

SRCROOT=$PWD

cd $SRCROOT/build/src/ffmpeg

git apply $SRCROOT/patch/ffmpeg/0001-create-lib-libraries.patch
git apply $SRCROOT/patch/ffmpeg/0002-fix-msvc-link.patch
git apply $SRCROOT/patch/ffmpeg/0003-fix-windowsinclude.patch
git apply $SRCROOT/patch/ffmpeg/0004-fix-debug-build.patch
git apply $SRCROOT/patch/ffmpeg/0005-fix-nasm.patch
git apply $SRCROOT/patch/ffmpeg/0006-fix-StaticFeatures.patch
git apply $SRCROOT/patch/ffmpeg/0007-fix-lib-naming.patch
git apply $SRCROOT/patch/ffmpeg/0009-Fix-fdk-detection.patch
git apply $SRCROOT/patch/ffmpeg/0011-Fix-x265-detection.patch
git apply $SRCROOT/patch/ffmpeg/0012-Fix-ssl-110-detection.patch
git apply $SRCROOT/patch/ffmpeg/0013-define-WINVER.patch
git apply $SRCROOT/patch/ffmpeg/0015-Fix-xml2-detection.patch
git apply $SRCROOT/patch/ffmpeg/0020-fix-aarch64-libswscale.patch
git apply $SRCROOT/patch/ffmpeg/0022-fix-iconv.patch

./configure --toolchain=msvc --arch=x86_64 --prefix=$SRCROOT/build \
--disable-programs \
--disable-doc \
--disable-asm \
--target-os=win32 \
--enable-runtime-cpudetect \
--enable-w32threads \
--enable-d3d11va \
--enable-dxva2 \
--enable-mediafoundation \
--enable-pic \
--enable-optimizations \
--enable-small \

make
make install
