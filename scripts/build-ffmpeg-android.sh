#!/bin/sh
# Builds the FFmpeg 6.1 shared libraries video_bink links against for one Android
# ABI (Bink video + Bink audio decoders and the Bink demuxer only) and drops them
# into lib/android/<cpu>/ next to the other prebuilts. Ported from
# Star1xr/csso-android-enhanced.
#
# usage: scripts/build-ffmpeg-android.sh armeabi-v7a|arm64-v8a
#
# Safe to call on every run: it does nothing when the libs are already in place,
# and reuses ffmpeg-out-<cpu> (cached by CI) before downloading the toolchain.
set -e

ABI=$1
case "$ABI" in
	armeabi-v7a)
		CPU=arm
		ARCH=arm
		NDK_ARCH=arch-arm
		# keep in sync with scripts/waifulib/xcompile.py (armeabi-v7a-hard)
		TRIPLE=arm-linux-androideabi21
		GCC_TOOLCHAIN=arm-linux-androideabi-4.9
		CPUFLAGS="-march=armv7-a -mcpu=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard"
		EXTRALIBS="-lm_hard"
		;;
	arm64-v8a)
		CPU=aarch64
		ARCH=aarch64
		# the NDK calls the 64 bit ARM sysroot arch-arm64, not arch-aarch64
		NDK_ARCH=arch-arm64
		TRIPLE=aarch64-linux-android21
		GCC_TOOLCHAIN=aarch64-linux-android-4.9
		CPUFLAGS=""
		EXTRALIBS=""
		;;
	*)
		echo "usage: $0 armeabi-v7a|arm64-v8a" >&2
		exit 1
		;;
esac

# the prebuilt dir lives in the `lib` submodule: it must be checked out before
# anything is written there, otherwise a later submodule update would refuse to
# clone into a non-empty directory.
git submodule init && git submodule update

DEST=lib/android/$CPU
FFMPEG_LIBS="avcodec avformat avutil avdevice avfilter swresample swscale"
SRC=ffmpeg-6.1
OUT=$PWD/ffmpeg-out-$CPU

copy_libs() {
	mkdir -p "$DEST"
	for lib in $FFMPEG_LIBS; do
		cp -L "$1/lib/lib$lib.so" "$DEST/"
	done
	ls -l "$DEST"
}

if [ -f "$DEST/libavcodec.so" ]; then
	echo "$DEST already contains ffmpeg, nothing to do"
	exit 0
fi

if [ -f "$OUT/lib/libavcodec.so" ]; then
	echo "reusing cached ffmpeg build in $OUT"
	copy_libs "$OUT"
	exit 0
fi

NDK=$PWD/android-ndk-r10e
LLVM_NAME=clang+llvm-11.1.0-x86_64-linux-gnu-ubuntu-16.04
LLVM=$PWD/$LLVM_NAME

if [ ! -d "$NDK" ]; then
	wget -q https://dl.google.com/android/repository/android-ndk-r10e-linux-x86_64.zip
	unzip -q android-ndk-r10e-linux-x86_64.zip && rm -f android-ndk-r10e-linux-x86_64.zip
fi
if [ ! -d "$LLVM" ]; then
	wget -q "https://github.com/llvm/llvm-project/releases/download/llvmorg-11.1.0/$LLVM_NAME.tar.xz"
	tar -xf "$LLVM_NAME.tar.xz" && rm -f "$LLVM_NAME.tar.xz"
fi

SYSROOT=$NDK/platforms/android-21/$NDK_ARCH
GCC=$NDK/toolchains/$GCC_TOOLCHAIN/prebuilt/linux-x86_64

CFLAGS="--target=$TRIPLE --sysroot=$SYSROOT --gcc-toolchain=$GCC $CPUFLAGS"
LDFLAGS="--target=$TRIPLE --sysroot=$SYSROOT --gcc-toolchain=$GCC -fuse-ld=lld $EXTRALIBS"

if [ ! -d "$SRC" ]; then
	wget -q "https://ffmpeg.org/releases/$SRC.tar.xz"
	tar -xf "$SRC.tar.xz"
fi

cd "$SRC"
./configure \
	--prefix="$OUT" \
	--target-os=android \
	--enable-cross-compile \
	--arch=$ARCH \
	--cc="$LLVM/bin/clang" \
	--cxx="$LLVM/bin/clang++" \
	--ar="$LLVM/bin/llvm-ar" \
	--nm="$LLVM/bin/llvm-nm" \
	--ranlib="$LLVM/bin/llvm-ranlib" \
	--strip="$LLVM/bin/llvm-strip" \
	--sysroot="$SYSROOT" \
	--extra-cflags="$CFLAGS -fPIC" \
	--extra-ldflags="$LDFLAGS" \
	--enable-pic \
	--enable-shared --disable-static \
	--disable-programs --disable-doc --disable-postproc \
	--disable-everything \
	--enable-decoder=bink,binkaudio_dct,binkaudio_rdft \
	--enable-demuxer=bink \
	--enable-protocol=file \
	--enable-avdevice --enable-avfilter --enable-swresample --enable-swscale \
	|| { echo "ffmpeg configure failed"; tail -n 120 ffbuild/config.log; exit 1; }

make -j"$(nproc)"
make install

cd ..
copy_libs "$OUT"
