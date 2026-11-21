#!/bin/sh

git submodule init && git submodule update
# CI downloads the NDK in a separate workflow step; this is the fallback for local builds.
if [ ! -d "android-ndk-r10e" ]; then
    wget https://dl.google.com/android/repository/android-ndk-r10e-linux-x86_64.zip
    unzip -q android-ndk-r10e-linux-x86_64.zip
fi
export ANDROID_NDK_HOME=$PWD/android-ndk-r10e/
export NDK_HOME=$PWD/android-ndk-r10e/

CCACHE_OPT=""
if command -v ccache >/dev/null 2>&1; then
    CCACHE_OPT="--use-ccache"
fi

LINKFLAGS="-Wl,--strip-all" CXXFLAGS="-fexceptions" CFLAGS="-fexceptions" ./waf configure -T release --android=armeabi-v7a-hard,4.9,21 --togles --disable-warns $CCACHE_OPT && ./waf build
