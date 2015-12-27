# Tunefish Synthesizer
This document describes compiling, installing and using the tunefish synth.

### Compiling on Windows

Compile using Visual Studio 2015. Solution can be found in
/src/tunefish4/Builds/VisualStudio2015

### Compiling on Mac OS X

Compile using XCode. Project can be found in
/src/tunefish4/Builds/MacOSX

### Compiling on Linux

The following additional packages are needed on Ubuntu 14.04 to compile:

* libgl1-mesa-dev
* libfreetype6-dev
* libasound-dev
* libxrandr-dev
* libxinerama-dev
* libxcursor-dev

run make in directory
/src/tunefish4/Builds/LinuxMakefile

### Running

Copy the directory tf4programs from /pathches/ and the tunefish4.dll/.so/.component/vst (depending on platform) to your desired plugins directory. Run your sequencer.
