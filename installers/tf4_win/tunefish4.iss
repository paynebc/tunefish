[Setup]
AppName=Tunefish4
AppVersion={#AppVersion}
DefaultDirName={commoncf}
DefaultGroupName=Tunefish
OutputBaseFilename=Tunefish4-{#AppVersion}
OutputDir=..\..\artifacts\
 
[Files]
Source: "..\..\build\Tunefish4.dll"; DestDir: "{app}\Steinberg\VST2"
Source: "..\..\build\Tunefish4.vst3"; DestDir: "{app}\Steinberg\VST3"
