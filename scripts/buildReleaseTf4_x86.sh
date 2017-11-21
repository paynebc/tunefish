#!/bin/sh
mkdir ../artifacts
mkdir ../build
cd ../build



tar cvfz ../../artifacts/tunefish4-$(cat ../../FULLVERSION_TF4)-x86.tar.gz *

cd ../../scripts