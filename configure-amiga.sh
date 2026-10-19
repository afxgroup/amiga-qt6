#!/bin/sh
# Configure the AmigaOS4 cross build of Qt 6.2: clib4 runtime, desktop OpenGL through mesa.library.
#
# Needs the host tools first (moc, rcc, uic, qmake), built from this same tree into
# build-host/install.
#
# Build:    cmake --build build-mesa --parallel
# Stage:    DESTDIR="$PWD/build-mesa/staging" cmake --install build-mesa
SRC="$(cd "$(dirname "$0")" && pwd)"
SDK=/usr/ppc-amigaos/SDK

export PKG_CONFIG_PATH="$SDK/local/clib4/lib/pkgconfig:$SDK/clib4/lib/pkgconfig"
cmake -S "$SRC" -B "$SRC/build-mesa" \
-G "Unix Makefiles" \
-DCMAKE_TOOLCHAIN_FILE="$SRC/toolchain-amigaos4-clib4.cmake" \
-DCMAKE_MAKE_PROGRAM=/usr/bin/make \
-DCMAKE_INSTALL_PREFIX="/qt6-amiga" \
-DQT_INSTALL_PREFIX="/qt6-amiga" \
-DQT_HOST_PATH="$SRC/build-host/install" \
-DQT_QMAKE_TARGET_MKSPEC=amiga-g++ \
-DUNIX=1 -DAMIGA=1 \
-DINPUT_opengl=desktop \
-DFEATURE_vulkan=OFF \
-DFEATURE_opengles2=OFF \
-DQT_FEATURE_dlopen=ON \
-DQT_FEATURE_thread=ON \
-DQT_FEATURE_network=ON \
-DQT_FEATURE_ssl=ON \
-DQT_FEATURE_openssl=ON \
-DQT_FEATURE_openssl_linked=OFF \
-DQT_FEATURE_library=ON \
-DQT_FEATURE_concurrent=ON \
-DQT_FEATURE_sql=ON \
-DQT_FEATURE_future=ON \
-DQT_FEATURE_process=ON \
-DQT_FEATURE_processenvironment=ON \
-DQT_FEATURE_systemsemaphore=ON \
-DQT_FEATURE_brotli=ON \
-DQT_FEATURE_icu=ON \
-DQT_FEATURE_libudev=OFF \
-DQT_FEATURE_evdev=OFF \
-DQT_FEATURE_pcre2=ON -DQT_FEATURE_system_pcre2=ON -DCMAKE_DISABLE_FIND_PACKAGE_PCRE2=ON \
-DQT_FEATURE_sql_odbc=OFF \
-DQT_FEATURE_system_sqlite=ON \
-DQT_FEATURE_qml_debug=OFF \
-DBUILD_SHARED_LIBS=OFF \
-DAMIGA_STATIC_BUILD=ON \
-DQT_BUILD_TOOLS_WHEN_CROSSCOMPILING=OFF \
-DINSTALL_QMLDIR=/qt6-amiga/qml \
-DINSTALL_LIBEXECDIR=libexec \
-DINSTALL_PLUGINSDIR=plugins \
-DINSTALL_DOCDIR=doc \
-DINSTALL_DESCRIPTIONSDIR=modules \
-DINSTALL_TRANSLATIONSDIR=translations
