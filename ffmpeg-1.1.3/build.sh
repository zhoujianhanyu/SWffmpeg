
export ANDROID_NDK=/home/zhoujian/Android/NDK/android-ndk-r8d/
export TOOLCHAIN=/home/zhoujian/Android/ffmpeg/toolchain
export SYSROOT=$TOOLCHAIN/sysroot/

rm -rf $TOOLCHAIN
$ANDROID_NDK/build/tools/make-standalone-toolchain.sh --platform=android-9 --install-dir=$TOOLCHAIN

export PATH=$TOOLCHAIN/bin:$PATH
export CC=arm-linux-androideabi-gcc
export LD=arm-linux-androideabi-ld
export AR=arm-linux-androideabi-ar

CFLAGCFLAGS="-O3 -Wall -mthumb -pipe -fpic -fasm \
  -finline-limit=300 -ffast-math \
  -fstrict-aliasing -Werror=strict-aliasing \
  -fmodulo-sched -fmodulo-sched-allow-regmoves \
  -Wno-psabi -Wa,--noexecstack \
  -D__ARM_ARCH_5__ -D__ARM_ARCH_5E__ \
  -D__ARM_ARCH_5T__ -D__ARM_ARCH_5TE__ \
  -DANDROID -DNDEBUG"

EXTRA_CFLAGS="-march=armv7-a -mfpu=neon -mfloat-abi=softfp -mvectorize-with-neon-quad"
EXTRA_LDFLAGS="-Wl,--fix-cortex-a8"

FFMPEG_FLAGS="--prefix=/tmp/ffmpeg/build \
  --target-os=linux \
  --arch=arm \
  --enable-cross-compile \
  --cross-prefix=arm-linux-androideabi- \
  --enable-shared \
  --enable-static \
  --disable-symver \
  --disable-doc \
  --disable-ffplay \
  --disable-ffmpeg \
  --disable-ffprobe \
  --disable-ffserver \
  --disable-avdevice \
  --disable-avfilter \
  --enable-encoders \
  --enable-muxers \
  --disable-filters \
  --disable-devices \
  --disable-everything \
  --enable-protocols  \
  --enable-parsers \
  --enable-demuxers \
  --disable-demuxer=sbg \
  --enable-decoders \
  --enable-bsfs \
  --enable-network \
  --enable-swscale  \
  --enable-asm \
  --enable-version3"

cd $1
./configure $FFMPEG_FLAGS --extra-cflags="$CFLAGS $EXTRA_CFLAGS" --extra-ldflags="$EXTRA_LDFLAGS"
make
cd ..
