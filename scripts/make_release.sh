#!/bin/bash

URL=https://github.com/smlu/ProjectMarduk.git 
BRANCH=develop

SOURCE_DIR="ProjectMarduk"

BUILD_DIR="$SOURCE_DIR/build"
WIN32="$BUILD_DIR/windows/x86"
WIN64="$BUILD_DIR/windows/x86_64"
UBUNTU64="$BUILD_DIR/ubuntu/x86_64"

MINGW_x86_CMAKE=mingw-w64-i686.cmake
MINGW_x86_64=mingw-w64-x86_64.cmake

OUTDIR="out"

BUILD_TYPE=MinSizeRel


if [ -d $SOURCE_DIR ] 
then
    pushd .
    cd $SOURCE_DIR
    git pull || exit 1
    popd
else
    git clone $URL -b $BRANCH --recursive $SOURCE_DIR || exit 1
fi


rm -rf $BUILD_DIR

mkdir $OUTDIR > /dev/null 2>&1

# Deterministic mingw builds
CXXFLAGS=-Wl,--no-insert-timestamp  cmake -S $SOURCE_DIR -B $WIN32 -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DCMAKE_TOOLCHAIN_FILE=$MINGW_x86_CMAKE
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi

cmake --build $WIN32 -- -j8
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi

CXXFLAGS=-Wl,--no-insert-timestamp cmake -S $SOURCE_DIR -B $WIN64 -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DCMAKE_TOOLCHAIN_FILE=$MINGW_x86_64
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi

cmake --build $WIN64 -- -j8
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi

# Deterministic clang build
CC=clang CXX=clang++ ZERO_AR_DATE=1 cmake -S $SOURCE_DIR -B $UBUNTU64 -DCMAKE_BUILD_TYPE=$BUILD_TYPE
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi

cmake --build $UBUNTU64 -- -j8
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi

# Compress and print hashes
zip -9jq "$OUTDIR/windows-x86.zip" "$WIN32/bin/cndtool.exe" "$WIN32/bin/gobext.exe" "$WIN32/bin/matool.exe"
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi

zip -9jq "$OUTDIR/windows-x86-64.zip" "$WIN64/bin/cndtool.exe" "$WIN64/bin/gobext.exe" "$WIN64/bin/matool.exe"
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi

tar -czf "$OUTDIR/linux-x86-64.tar.gz" -C "$UBUNTU64/bin/" cndtool gobext matool > /dev/null
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi


echo "sha256 windows-x86.zip: $(sha256sum $OUTDIR/windows-x86.zip | awk '{ print $1 }')"
echo "sha256 windows-x86-64.zip: $(sha256sum $OUTDIR/windows-x86-64.zip | awk '{ print $1 }')"
echo "sha256 linux-x86-64.tar.gz: $(sha256sum $OUTDIR/linux-x86-64.tar.gz | awk '{ print $1 }')"

