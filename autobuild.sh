#! /bin/bash

set -e

rm -rf /build/*
cd `pwd`/build &&
    cmake .. &&
    make
cd ..
cp -r `pwd`/src/include `pwd`/lib