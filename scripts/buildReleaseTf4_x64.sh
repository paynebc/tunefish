#!/bin/sh
mkdir ../artifacts

cd ../src/tunefish4/Builds/LinuxMakefile

tar cvfz ../../../../artifacts/tunefish4-$(cat ../../FULLVERSION_TF4)-x64.tar.gz *

cd ../../../../scripts