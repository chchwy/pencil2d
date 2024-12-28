#!/bin/sh -l

set -e
tree
ldd --version
git --version
curl --version

export MAKEFLAGS=-j2
export LANG=C.UTF-8

ls /opt/qt515/bin
source /opt/qt515/bin/qt515-env.sh

qmake --version

echo "Hello $1"
time=$(date)
echo "time=$time" >> $GITHUB_OUTPUT

qmake 
make
