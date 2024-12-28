#!/bin/sh -l

set -e
ldd --version
git --version
curl --version


export LANG=C.UTF-8

QT_BASE_DIR=/opt/qt515
export QTDIR=$QT_BASE_DIR
export PATH=$QT_BASE_DIR/bin:$PATH

if [ $(uname -m) = "x86_64" ]; then
  export LD_LIBRARY_PATH=$QT_BASE_DIR/lib/x86_64-linux-gnu:$QT_BASE_DIR/lib:$LD_LIBRARY_PATH
else
  export LD_LIBRARY_PATH=$QT_BASE_DIR/lib/i386-linux-gnu:$QT_BASE_DIR/lib:$LD_LIBRARY_PATH
fi
export PKG_CONFIG_PATH=$QT_BASE_DIR/lib/pkgconfig:$PKG_CONFIG_PATH

qmake --version
qmake -o build PREFIX=/usr CONFIG-=debug_and_release CONFIG+=release CONFIG+=GIT CONFIG+=PENCIL2D_NIGHTLY VERSION=1.0.0
make -j2
tree
