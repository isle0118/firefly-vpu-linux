#!/bin/bash

LOCAL_PATH=`pwd`
CFLAGS="-O3 -Wall -marm -pipe -fpic -fasm \
  -finline-limit=300 -ffast-math \
  -fstrict-aliasing -Werror=strict-aliasing \
  -fmodulo-sched -fmodulo-sched-allow-regmoves \
  -Wno-psabi -Wa,--noexecstack -I./prebuild/include"
LDFLAGS="-ldl -L./prebuild/libs"

EXTRA_CFLAGS=

EXTRA_LDFLAGS=

FFMPEG_FLAGS="--prefix=/usr/local \
  --target-os=linux \
  --arch=arm \
  --enable-cross-compile \
  --cross-prefix=arm-linux-gnueabihf- \
  --enable-shared \
  --disable-symver \
  --disable-doc \
  --enable-ffplay \
  --disable-ffmpeg \
  --disable-ffprobe \
  --disable-ffserver \
  --disable-avdevice \
  --disable-avfilter \
  --disable-encoders  \
  --disable-muxers \
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
  --enable-neon \
  --enable-asm \
  --enable-debug \
  --enable-version3 "

#--disable-stripping \
#--enable-asm

./configure $FFMPEG_FLAGS --extra-cflags="$CFLAGS $EXTRA_CFLAGS" --extra-ldflags="$LDFLAGS $EXTRA_LDFLAGS"

make -j8 && make DESTDIR=out install

cp -rf ./prebuild/include/* out/usr/local/include
cp -rf ./prebuild/libs/* out/usr/local/lib
