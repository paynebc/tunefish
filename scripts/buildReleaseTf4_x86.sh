#!/bin/sh
mkdir ../artifacts
mkdir ../build
cd ../build

cp ../src/tunefish4/Builds/LinuxMakefile/build/Tunefish4.so .
cp ../README.txt .
cp ../CHANGES.txt .
cp ../COPYING .

tar cvfz ../artifacts/tunefish4-$(cat ../FULLVERSION_TF4)-x86.tar.gz *

cd ../scripts