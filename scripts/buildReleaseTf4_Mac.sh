#!/bin/sh
mkdir ../artifacts
mkdir ../build
mkdir ../build/dmg
cd ../build

cp -R ~/Library/Audio/Plug-Ins/Components/Tunefish4.component .
cp -R ~/Library/Audio/Plug-Ins/VST/Tunefish4.vst .
cp -R ~/Library/Audio/Plug-Ins/VST3/Tunefish4.vst3 .
cp ../README.txt .
cp ../CHANGES.txt .
cp ../COPYING .

zip -9 -r ../artifacts/tunefish4-$(cat ../FULLVERSION_TF4).zip *

packagesbuild ../installers/tf4_mac/Tunefish\ 4.pkgproj
cp ../installers/tf4_mac/build/Tunefish4.pkg ../build/dmg/tunefish4-$(cat ../FULLVERSION_TF4).pkg
cp ../README.txt ../build/dmg
cp ../CHANGES.txt ../build/dmg
cp ../COPYING ../build/dmg

hdiutil create ../build/tmp.dmg -ov -volname "Tunefish4" -fs HFS+ -srcfolder ../build/dmg
hdiutil convert ../build/tmp.dmg -format UDZO -o ../artifacts/tunefish4-$(cat ../FULLVERSION_TF4).dmg

cd ../scripts