[Setup]
AppName=Tunefish4
AppVersion={#AppVersion}
WizardStyle=modern
DefaultDirName={commoncf}
DefaultGroupName=Tunefish
Compression=lzma2
OutputBaseFilename=Tunefish4-{#AppVersion}
OutputDir=..\..\artifacts\
LicenseFile=..\..\COPYING
 
[Files]
Source: "..\..\build\Tunefish4.dll"; DestDir: "{app}\Steinberg\VST2"
Source: "..\..\build\Tunefish4.vst3"; DestDir: "{app}\Steinberg\VST3"
