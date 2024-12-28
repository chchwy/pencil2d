#!/bin/sh -l

set -e
tree
ldd --version
git --version
curl --version

export MAKEFLAGS=-j2
# Our container image uses the non-Unicode C locale by default
export LANG=C.UTF-8
# Set up Qt environment variables and export them to the GitHub Actions workflow
sh /opt/qt515/bin/qt515-env.sh
qmake --version

echo "Hello $1"
time=$(date)
echo "time=$time" >> $GITHUB_OUTPUT

qmake 
make
