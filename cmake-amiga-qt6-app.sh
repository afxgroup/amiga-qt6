#!/bin/sh
# Configure an application for AmigaOS4 against the Qt 6.2 build (clib4, mesa.library).
# Run from the application source directory, pointing QT_PREFIX at the installed Qt
# (the staged install, e.g. build-mesa/staging/qt6-amiga):
#   QT_PREFIX=/path/to/qt6-amiga sh /path/to/cmake-amiga-qt6-app.sh
# Then:  cmake --build build-app --parallel
SDK=/usr/ppc-amigaos/SDK
QT_PREFIX="${QT_PREFIX:-/qt6-amiga}"
QT_HOST="${QT_HOST:-$(cd "$(dirname "$0")" && pwd)/build-host/install}"

"$QT_PREFIX/bin/qt-cmake" -S . -B build-app \
-DCMAKE_FIND_ROOT_PATH="$SDK;$SDK/clib4;$SDK/local/clib4;$SDK/local/common" \
-DCMAKE_FIND_ROOT_PATH_MODE_INCLUDE=ONLY \
-DCMAKE_LIBRARY_PATH="$SDK/clib4/lib;$SDK/local/clib4/lib;$SDK/local/common/lib" \
-DCMAKE_INCLUDE_PATH="$SDK/local;$SDK/clib4/include;$SDK/local/common/include;$SDK/local/clib4/include;$SDK/include/include_h" \
-DCMAKE_PREFIX_PATH="$SDK;$SDK/clib4;$SDK/local;$SDK/local/clib4;$SDK/local/common" \
-DCMAKE_SYSTEM_NAME="AmigaOS" \
-DCMAKE_SYSTEM_PROCESSOR="PowerPC" \
-DCMAKE_C_COMPILER="ppc-amigaos-gcc" \
-DCMAKE_CXX_COMPILER="ppc-amigaos-g++" \
-DCMAKE_ASM_COMPILER="ppc-amigaos-as" \
-DCMAKE_MAKE_PROGRAM="make" \
-DCMAKE_CXX_FLAGS_INIT="-mcrt=clib4 -athread=native -I$SDK/local" \
-DCMAKE_C_FLAGS_INIT="-mcrt=clib4 -athread=native" \
-DCMAKE_EXE_LINKER_FLAGS="-mcrt=clib4 -athread=native -Wl,-Bstatic" \
-DUNIX=1 -DAMIGA=1 \
-DQT_INSTALL_PREFIX="$QT_PREFIX" \
-DCMAKE_INSTALL_PREFIX="$QT_PREFIX" \
-DQT_HOST_PATH="$QT_HOST" \
-DQT_QMAKE_TARGET_MKSPEC=amiga-g++ \
-DCMAKE_DISABLE_FIND_PACKAGE_PCRE2=ON \
-DCMAKE_FIND_LIBRARY_SUFFIXES=.a \
-DZLIB_LIBRARY="$SDK/local/clib4/lib/libz.a" \
-DZLIB_INCLUDE_DIR="$SDK/local/clib4/include" \
-DPCRE2_LIBRARY_RELEASE="$SDK/local/clib4/lib/libpcre2-16.a" \
-DPCRE2_LIBRARY_DEBUG="$SDK/local/clib4/lib/libpcre2-16.a" \
-DCMAKE_DL_LIBS="" \
-DJPEG_LIBRARY="$SDK/local/clib4/lib/libjpeg.a" \
-DJPEG_INCLUDE_DIR="$SDK/local/clib4/include"
