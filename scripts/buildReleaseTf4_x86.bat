@echo off
set /p VER=<..\FULLVERSION_TF4
mkdir ..\artifacts
mkdir ..\build
cd ..\build

xcopy ..\binary\x86\VST\Tunefish4.dll . /v /y
xcopy ..\binary\x86\VST3\Tunefish4.vst3 . /v /y
xcopy ..\README.txt . /v /y
xcopy ..\COPYING . /v /y
xcopy ..\CHANGES.txt . /v /y 

"C:\Program Files (x86)\Inno Setup 6\iscc" /dAppVersion="%VER%-x86" ..\installers\tf4_win\Tunefish4.iss
zip ..\artifacts\tunefish4-%VER%-x86.zip . -r
cd ..\scripts