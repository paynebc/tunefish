#!/bin/sh
mkdir ../artifacts
mkdir ../build
cd ../build

cp -R ~/Library/Audio/Plug-Ins/Components/Tunefish4.component .
cp -R ~/Library/Audio/Plug-Ins/VST/Tunefish4.vst .
cp -R ~/Library/Audio/Plug-Ins/VST3/Tunefish4.vst3 .
cp ../README.txt .
cp ../CHANGES.txt .
cp ../COPYING .

zip -9 -r ../artifacts/tunefish4-$(cat ../FULLVERSION_TF4)-x64.zip *

cd ../scripts