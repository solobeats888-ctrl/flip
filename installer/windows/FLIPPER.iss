; Instalador de FLIPPER para Windows (VST3, 64 bits)
#ifndef SourceDir
  #define SourceDir "..\..\build\Flipper_artefacts\Release\VST3\FLIPPER.vst3"
#endif

[Setup]
AppId={{8C2E4F71-3A9D-4B6E-A5C1-F11990000001}
AppName=FLIPPER
AppVersion=0.9.1
AppVerName=FLIPPER 0.9.1 (Viszeversza)
AppPublisher=Viszeversza
DefaultDirName={commoncf64}\VST3
DisableDirPage=yes
DisableProgramGroupPage=yes
DisableReadyPage=no
OutputBaseFilename=FLIPPER-Instalador-Windows
ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64
PrivilegesRequired=admin
UninstallFilesDir={commonappdata}\Viszeversza\FLIPPER
UninstallDisplayName=FLIPPER (VST3) - Viszeversza
Compression=lzma2
SolidCompression=yes
WizardStyle=modern

[Languages]
Name: "spanish"; MessagesFile: "compiler:Languages\Spanish.isl"
Name: "english"; MessagesFile: "compiler:Default.isl"

[InstallDelete]
Type: filesandordirs; Name: "{commoncf64}\VST3\FLIPPER.vst3"

[UninstallDelete]
Type: filesandordirs; Name: "{commoncf64}\VST3\FLIPPER.vst3"

[Files]
Source: "{#SourceDir}\*"; DestDir: "{commoncf64}\VST3\FLIPPER.vst3"; Flags: ignoreversion recursesubdirs createallsubdirs

[Messages]
spanish.WelcomeLabel1=FLIPPER — motor de síntesis vintage, por Viszeversza
spanish.WelcomeLabel2=Esto instala FLIPPER (VST3, 64 bits) en la carpeta de plugins VST3 de Windows.%n%nCerrá FL Studio antes de seguir.
spanish.FinishedLabel=FLIPPER quedó instalado. Abrí FL Studio, actualizá la lista de plugins y cargalo como instrumento en el Channel Rack.
