#!/bin/sh
mkdir ../artifacts
mkdir ../build
cd ../build



tar cvfz ../../artifacts/tunefish4-$(cat ../../FULLVERSION_TF4)-x64.tar.gz *

cd ../../scripts