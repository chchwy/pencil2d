#!/bin/sh -l

set -e
tree
ldd --version
git --version
curl --version

export MAKEFLAGS=-j2
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

echo "Hello $1"
time=$(date)
echo "time=$time" >> $GITHUB_OUTPUT

qmake 
make
