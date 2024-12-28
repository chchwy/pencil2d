#!/bin/sh -l

echo "Hello $1"
time=$(date)
echo "time=$time" >> $GITHUB_OUTPUT


tree
ldd --version
git --version
curl --version
qmake --version
