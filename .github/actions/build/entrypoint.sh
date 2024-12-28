#!/bin/sh -l

set -e
tree
ldd --version
git --version
curl --version
qmake --version


echo "Hello $1"
time=$(date)
echo "time=$time" >> $GITHUB_OUTPUT

qmake 
make
