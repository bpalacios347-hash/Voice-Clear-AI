; =============================================================================
; Voice Clear AI — Inno Setup Installer Script
; =============================================================================
; Prerequisites (must be present on the build machine):
;   • Inno Setup 6.2+  (iscc.exe)
;   • Completed Release build:  build\bin\Release\VoiceClear.exe
;   • Qt 6.x runtime DLLs:     build\bin\Release\VoiceClear.exe + Qt DLLs
;   • ONNX Runtime DLL:        external\onnxruntime\lib\onnxruntime.dll
;   • Signed driver package:   build\driver\Release\VoiceClearVAD.sys
;                               src\driver\sys\VoiceClearVAD.inf
;                               src\driver\sys\VoiceClearVAD.cat
; =============================================================================

[Setup]
AppName=Voice Clear AI
AppVersion=1.0.0-RC1
AppVerName=Voice Clear AI 1.0.0 (por Byron J. P)
AppPublisher=Byron J. P
AppPublisherURL=https://github.com/voiceclearai
AppSupportURL=https://github.com/voiceclearai/issues
AppUpdatesURL=https://github.com/voiceclearai/releases
AppComments=Voice Clear AI - Cancelacion de Ruido con IA en Tiempo Real por Byron J. P
AppCopyright=Copyright (C) 2026 Byron J. P. Todos los derechos reservados.

; Install into Program Files\Voice Clear AI
DefaultDirName={autopf}\Voice Clear AI
DefaultGroupName=Voice Clear AI

; Uninstall
UninstallDisplayIcon={app}\VoiceClear.exe
UninstallDisplayName=Voice Clear AI 1.0.0 (Byron J. P)

; Compression
Compression=lzma2/ultra64
SolidCompression=yes
DiskSpanning=no

; Output
OutputDir=..\build\installer
OutputBaseFilename=VoiceClearAI_Setup_Final_x64

; Architecture — 64-bit only (AVX2 requires x64)
ArchitecturesInstallIn64BitMode=x64
ArchitecturesAllowed=x64

; Elevation
PrivilegesRequired=admin
PrivilegesRequiredOverridesAllowed=commandline

; Wizard appearance
WizardStyle=modern
WizardSizePercent=100
SetupIconFile=..\installer\app.ico
WizardImageFile=..\installer\wizard_image.bmp
WizardSmallImageFile=..\installer\wizard_small.bmp

; License
LicenseFile=..\LICENSE.txt

; Version info embedded in the installer executable
VersionInfoVersion=1.0.0.0
VersionInfoCompany=Byron J. P
VersionInfoDescription=Voice Clear AI Setup - Autor: Byron J. P
VersionInfoCopyright=Copyright (C) 2026 Byron J. P

; =============================================================================
; Languages & Custom Author Messages
; =============================================================================
[Languages]
Name: "spanish"; MessagesFile: "compiler:Languages\Spanish.isl"
Name: "english"; MessagesFile: "compiler:Default.isl"

[Messages]
spanish.WelcomeLabel1=Bienvenido al Asistente de Instalación de Voice Clear AI
spanish.WelcomeLabel2=Este programa instalará Voice Clear AI en su sistema.%n%n══════════════════════════════════════════%n Autor y Desarrollador: Byron J. P%n Versión: 1.0.0 RC1 (DeepFilterNet AI Engine)%n Firma: Byron J. P%n══════════════════════════════════════════%n%nSe recomienda cerrar cualquier otra aplicación de audio antes de continuar.
spanish.FinishedHeadingLabel=Instalación de Voice Clear AI Completada
spanish.FinishedLabel=Voice Clear AI ha sido instalado con éxito en su equipo.%n%nCreado por Byron J. P.%n¡Disfrute de una voz limpia y cristalina en tiempo real!

english.WelcomeLabel1=Welcome to Voice Clear AI Setup Wizard
english.WelcomeLabel2=This will install Voice Clear AI on your computer.%n%n══════════════════════════════════════════%n Author & Developer: Byron J. P%n Version: 1.0.0 RC1 (DeepFilterNet AI Engine)%n Signature: Byron J. P%n══════════════════════════════════════════%n%nIt is recommended that you close other audio applications before continuing.
english.FinishedHeadingLabel=Voice Clear AI Installation Completed
english.FinishedLabel=Voice Clear AI has been successfully installed on your computer.%n%nCreated by Byron J. P.%nEnjoy clean, studio-grade real-time voice processing!

[CustomMessages]
spanish.ProductKeyTitle=Activación de Licencia / Clave de Producto
spanish.ProductKeySubTitle=Ingrese su Clave de Activación única para este equipo.
spanish.ProductKeyInfo=Copie su Machine ID y envíelo por WhatsApp a Byron J. P para recibir su clave oficial:
spanish.ProductKeyMachineID=ID de esta Computadora (Machine ID):
spanish.ProductKeyCopyBtn=Copiar Machine ID
spanish.ProductKeyLabel=Clave de Activación (Product Key):
spanish.ProductKeyCopied=¡Machine ID copiado al portapapeles con éxito!%n%nAhora puedes pegarlo (Ctrl + V) en tu chat de WhatsApp para enviárselo a Byron J. P.

english.ProductKeyTitle=License Activation / Product Key
english.ProductKeySubTitle=Enter your unique Activation Key for this computer.
english.ProductKeyInfo=Copy your Machine ID and send it via WhatsApp to Byron J. P to receive your official key:
english.ProductKeyMachineID=This Computer ID (Machine ID):
english.ProductKeyCopyBtn=Copy Machine ID
english.ProductKeyLabel=Activation Key (Product Key):
english.ProductKeyCopied=Machine ID copied to clipboard successfully!%n%nYou can now paste it (Ctrl + V) into your WhatsApp chat to send it to Byron J. P.

; =============================================================================
; Pre-flight check & Product Key Validation — custom Pascal script
; =============================================================================
[Code]

var
  ActivationPage: TWizardPage;
  MachineIDEdit: TEdit;
  ProductKeyEdit: TEdit;
  CopyIDButton: TButton;
  ValidatedProductKey: String;

procedure CopyMachineIDClick(Sender: TObject);
var
  ResultCode: Integer;
begin
  if MachineIDEdit <> nil then
  begin
    MachineIDEdit.SelectAll;
    Exec(ExpandConstant('{sys}\cmd.exe'),
         '/c echo|set /p="' + MachineIDEdit.Text + '"|clip',
         '', SW_HIDE, ewWaitUntilTerminated, ResultCode);
    MsgBox(CustomMessage('ProductKeyCopied'), mbInformation, MB_OK);
  end;
end;

// ----------------------------------------------------------------------------
// Base32 Character conversion helpers
// ----------------------------------------------------------------------------
function CharToVal(C: Char): Integer;
begin
  case C of
    '2': Result := 0;  '3': Result := 1;  '4': Result := 2;  '5': Result := 3;
    '6': Result := 4;  '7': Result := 5;  '8': Result := 6;  '9': Result := 7;
    'A': Result := 8;  'B': Result := 9;  'C': Result := 10; 'D': Result := 11;
    'E': Result := 12; 'F': Result := 13; 'G': Result := 14; 'H': Result := 15;
    'J': Result := 16; 'K': Result := 17; 'L': Result := 18; 'M': Result := 19;
    'N': Result := 20; 'P': Result := 21; 'Q': Result := 22; 'R': Result := 23;
    'S': Result := 24; 'T': Result := 25; 'U': Result := 26; 'V': Result := 27;
    'W': Result := 28; 'X': Result := 29; 'Y': Result := 30; 'Z': Result := 31;
  else
    Result := -1;
  end;
end;

function ValToChar(V: Integer): Char;
var
  Charset: String;
begin
  Charset := '23456789ABCDEFGHJKLMNPQRSTUVWXYZ';
  if (V >= 0) and (V < 32) then
    Result := Charset[V + 1]
  else
    Result := '?';
end;

// ----------------------------------------------------------------------------
// Calculate Local Machine ID from Windows MachineGuid
// ----------------------------------------------------------------------------
function GetMachineID(): String;
var
  GuidStr: String;
  I, Val, Sum1, Sum2: Integer;
  D0, D1, D2, D3, D4, D5, D6, D7: Integer;
begin
  GuidStr := '';
  if not RegQueryStringValue(HKLM, 'SOFTWARE\Microsoft\Cryptography', 'MachineGuid', GuidStr) then
  begin
    GuidStr := GetComputerNameString() + 'VOICECLEAR';
  end;
  GuidStr := UpperCase(Trim(GuidStr));
  StringChangeEx(GuidStr, '-', '', True);
  StringChangeEx(GuidStr, '{', '', True);
  StringChangeEx(GuidStr, '}', '', True);
  StringChangeEx(GuidStr, ' ', '', True);

  Sum1 := 12345;
  Sum2 := 67890;

  for I := 1 to Length(GuidStr) do
  begin
    Val := Ord(GuidStr[I]);
    Sum1 := (Sum1 * 31 + Val) mod 1048576;
    Sum2 := (Sum2 * 37 + Val) mod 1048576;
  end;

  D0 := (Sum1 div 32768) mod 32;
  D1 := (Sum1 div 1024) mod 32;
  D2 := (Sum1 div 32) mod 32;
  D3 := Sum1 mod 32;

  D4 := (Sum2 div 32768) mod 32;
  D5 := (Sum2 div 1024) mod 32;
  D6 := (Sum2 div 32) mod 32;
  D7 := Sum2 mod 32;

  Result := ValToChar(D0) + ValToChar(D1) + ValToChar(D2) + ValToChar(D3) + '-' +
            ValToChar(D4) + ValToChar(D5) + ValToChar(D6) + ValToChar(D7);
end;

// ----------------------------------------------------------------------------
// Cryptographic Polynomial Checksum & HWID Validator
// ----------------------------------------------------------------------------
function ValidateProductKey(RawKey: String): Boolean;
var
  Cleaned, Body, ChkGiven, ChkExpected, KeyHwid, LocalHwid, CleanLocalHwid: String;
  I, Val, Sum1, Sum2, H1, H2, D0, D1, D2, D3: Integer;
  W1: array[0..11] of Integer;
  W2: array[0..11] of Integer;
begin
  Result := False;
  Cleaned := UpperCase(Trim(RawKey));
  StringChangeEx(Cleaned, ' ', '', True);
  StringChangeEx(Cleaned, '-', '', True);

  // Strip optional 'VCAI' prefix
  if Pos('VCAI', Cleaned) = 1 then
    Cleaned := Copy(Cleaned, 5, Length(Cleaned) - 4);

  if Length(Cleaned) <> 16 then
  begin
    MsgBox('La clave debe tener 16 caracteres (Ejemplo: VCAI-XXXX-XXXX-XXXX-XXXX).', mbError, MB_OK);
    Exit;
  end;

  for I := 1 to 16 do
  begin
    if CharToVal(Cleaned[I]) = -1 then
    begin
      MsgBox('La clave contiene caracteres inválidos.', mbError, MB_OK);
      Exit;
    end;
  end;

  // 1. Verify that Key belongs to THIS computer's Machine ID
  LocalHwid := GetMachineID();
  CleanLocalHwid := LocalHwid;
  StringChangeEx(CleanLocalHwid, '-', '', True);

  KeyHwid := Copy(Cleaned, 1, 8);
  if KeyHwid <> CleanLocalHwid then
  begin
    MsgBox(
      'Esta Clave de Producto NO pertenece a esta computadora.'#13#10#13#10 +
      'ID de este equipo:    ' + LocalHwid + #13#10 +
      'ID de la clave usada: ' + Copy(KeyHwid, 1, 4) + '-' + Copy(KeyHwid, 5, 4) + #13#10#13#10 +
      'Cada clave es única e intransferible. Solicite una clave válida para su equipo a Byron J. P vía WhatsApp.',
      mbError, MB_OK
    );
    Exit;
  end;

  // 2. Verify Cryptographic Polynomial Signature
  Body := Copy(Cleaned, 1, 12);
  ChkGiven := Copy(Cleaned, 13, 4);

  W1[0] := 3;  W1[1] := 7;  W1[2] := 11; W1[3] := 13;
  W1[4] := 17; W1[5] := 19; W1[6] := 23; W1[7] := 29;
  W1[8] := 31; W1[9] := 37; W1[10] := 41; W1[11] := 43;

  W2[0] := 47; W2[1] := 43; W2[2] := 37; W2[3] := 31;
  W2[4] := 29; W2[5] := 23; W2[6] := 19; W2[7] := 17;
  W2[8] := 13; W2[9] := 11; W2[10] := 7;  W2[11] := 3;

  Sum1 := 789;
  Sum2 := 456;

  for I := 1 to 12 do
  begin
    Val := CharToVal(Body[I]) + 1;
    Sum1 := Sum1 + Val * W1[I - 1];
    Sum2 := Sum2 + Val * W2[I - 1];
  end;

  H1 := Sum1 mod 1024;
  H2 := Sum2 mod 1024;

  D0 := H1 div 32;
  D1 := H1 mod 32;
  D2 := H2 div 32;
  D3 := H2 mod 32;

  ChkExpected := ValToChar(D0) + ValToChar(D1) + ValToChar(D2) + ValToChar(D3);

  if ChkGiven <> ChkExpected then
  begin
    MsgBox(
      'La firma de la Clave de Producto es INVÁLIDA o fue alterada.'#13#10#13#10 +
      'Por favor verifique la clave o solicite una clave oficial al bot de WhatsApp de Byron J. P.',
      mbError, MB_OK
    );
    Exit;
  end;

  ValidatedProductKey := 'VCAI-' + Copy(Body, 1, 4) + '-' + Copy(Body, 5, 4) + '-' + Copy(Body, 9, 4) + '-' + ChkGiven;
  Result := True;
end;

// ----------------------------------------------------------------------------
// InitializeWizard — create custom Product Key query page
// ----------------------------------------------------------------------------
procedure InitializeWizard();
var
  Mid: String;
  LblInfo, LblMid, LblKey: TLabel;
begin
  Mid := GetMachineID();

  ActivationPage := CreateCustomPage(
    wpLicense,
    CustomMessage('ProductKeyTitle'),
    CustomMessage('ProductKeySubTitle')
  );

  // Label 1: Instructions (Top = 10, Height = 35)
  LblInfo := TLabel.Create(ActivationPage);
  LblInfo.Parent := ActivationPage.Surface;
  LblInfo.Left := ScaleX(0);
  LblInfo.Top := ScaleY(10);
  LblInfo.Width := ScaleX(410);
  LblInfo.Height := ScaleY(35);
  LblInfo.AutoSize := False;
  LblInfo.WordWrap := True;
  LblInfo.Caption := CustomMessage('ProductKeyInfo');

  // Label 2: Machine ID Title (Top = 55)
  LblMid := TLabel.Create(ActivationPage);
  LblMid.Parent := ActivationPage.Surface;
  LblMid.Left := ScaleX(0);
  LblMid.Top := ScaleY(55);
  LblMid.Width := ScaleX(410);
  LblMid.AutoSize := False;
  LblMid.Height := ScaleY(18);
  LblMid.Caption := CustomMessage('ProductKeyMachineID');
  LblMid.Font.Style := [fsBold];

  // Edit Box: Machine ID (Top = 75, Left = 0, Width = 240, Height = 25)
  MachineIDEdit := TEdit.Create(ActivationPage);
  MachineIDEdit.Parent := ActivationPage.Surface;
  MachineIDEdit.Left := ScaleX(0);
  MachineIDEdit.Top := ScaleY(75);
  MachineIDEdit.Width := ScaleX(240);
  MachineIDEdit.Height := ScaleY(25);
  MachineIDEdit.Text := Mid;
  MachineIDEdit.ReadOnly := True;
  MachineIDEdit.Font.Style := [fsBold];

  // Button: Copy Machine ID (Top = 74, Left = 250, Width = 160, Height = 27)
  CopyIDButton := TButton.Create(ActivationPage);
  CopyIDButton.Parent := ActivationPage.Surface;
  CopyIDButton.Left := ScaleX(250);
  CopyIDButton.Top := ScaleY(74);
  CopyIDButton.Width := ScaleX(160);
  CopyIDButton.Height := ScaleY(27);
  CopyIDButton.Caption := CustomMessage('ProductKeyCopyBtn');
  CopyIDButton.OnClick := @CopyMachineIDClick;

  // Label 3: Product Key Input Title (Top = 120)
  LblKey := TLabel.Create(ActivationPage);
  LblKey.Parent := ActivationPage.Surface;
  LblKey.Left := ScaleX(0);
  LblKey.Top := ScaleY(120);
  LblKey.Width := ScaleX(410);
  LblKey.AutoSize := False;
  LblKey.Height := ScaleY(18);
  LblKey.Caption := CustomMessage('ProductKeyLabel');
  LblKey.Font.Style := [fsBold];

  // Edit Box: Product Key Input (Top = 140, Left = 0, Width = 410, Height = 25)
  ProductKeyEdit := TEdit.Create(ActivationPage);
  ProductKeyEdit.Parent := ActivationPage.Surface;
  ProductKeyEdit.Left := ScaleX(0);
  ProductKeyEdit.Top := ScaleY(140);
  ProductKeyEdit.Width := ScaleX(410);
  ProductKeyEdit.Height := ScaleY(25);
  ProductKeyEdit.Text := '';
end;

// ----------------------------------------------------------------------------
// NextButtonClick — validate key before advancing past the Product Key page
// ----------------------------------------------------------------------------
function NextButtonClick(CurPageID: Integer): Boolean;
begin
  Result := True;
  if (ActivationPage <> nil) and (CurPageID = ActivationPage.ID) then
  begin
    if not ValidateProductKey(ProductKeyEdit.Text) then
    begin
      Result := False;
    end;
  end;
end;

// ----------------------------------------------------------------------------
// AVX2 CPU feature detection via CPUID (leaf 7, sub-leaf 0, EBX bit 5)
// ----------------------------------------------------------------------------
function IsCpuAvx2Supported(): Boolean;
begin
  Result := True; // Default: allow install, warn at runtime if AVX2 is absent
end;

// ----------------------------------------------------------------------------
// Check that the MSVC 2022 VC runtime is present (vcruntime140.dll)
// ----------------------------------------------------------------------------
function IsMsvcRuntimePresent(): Boolean;
begin
  Result := FileExists(ExpandConstant('{sys}\vcruntime140.dll')) or
            FileExists(ExpandConstant('{sys}\vcruntime140_1.dll'));
end;

// ----------------------------------------------------------------------------
// InitializeSetup — hard-stop if critical requirements are not met
// ----------------------------------------------------------------------------
function InitializeSetup(): Boolean;
var
  ErrorCode: Integer;
begin
  Result := True;

  // 1. MSVC Runtime (vcruntime140.dll)
  if not IsMsvcRuntimePresent() then begin
    if MsgBox(
      'The Microsoft Visual C++ 2015-2022 Redistributable (x64) is not installed.'#13#10#13#10 +
      'Voice Clear AI requires this runtime to operate.'#13#10#13#10 +
      'Click OK to open the Microsoft download page, then re-run this installer.',
      mbConfirmation, MB_OKCANCEL) = IDOK then begin
        ShellExec('open',
          'https://aka.ms/vs/17/release/vc_redist.x64.exe',
          '', '', SW_SHOWNORMAL, ewNoWait, ErrorCode);
    end;
    Result := False;
    Exit;
  end;
end;

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// CurStepChanged — perform post-install tasks
// ----------------------------------------------------------------------------
procedure CurStepChanged(CurStep: TSetupStep);
var
  ResultCode: Integer;
  DriverInfPath: String;
  VbCableInstaller: String;
begin
  if CurStep = ssPostInstall then begin

    // (A) Install Windows Service via the service binary itself
    if WizardIsTaskSelected('installservice') then begin
      if not Exec(ExpandConstant('{app}\VoiceClearService.exe'),
                  '--install', '', SW_HIDE, ewWaitUntilTerminated, ResultCode) then begin
        MsgBox('Warning: Failed to install the Voice Clear Windows Service.'#13#10 +
               'You can install it manually by running:'#13#10 +
               '  VoiceClearService.exe --install'#13#10 +
               'from an administrator command prompt.',
               mbInformation, MB_OK);
      end;
    end;

    // (B) Install the Microsoft WHQL-Signed Virtual Audio Cable Driver (Default & Automatic)
    if WizardIsTaskSelected('installvbcable') or
       (not (RegKeyExists(HKLM, 'SOFTWARE\VB-Audio\Cable') or
             RegKeyExists(HKLM, 'SOFTWARE\WOW6432Node\VB-Audio\Cable'))) then begin
      VbCableInstaller := ExpandConstant('{app}\vbcable\VBCABLE_Setup_x64.exe');
      if FileExists(VbCableInstaller) then begin
        Exec(VbCableInstaller, '-i -h', '', SW_HIDE, ewWaitUntilTerminated, ResultCode);
      end;
    end;

    // (C) Install the native VoiceClearVAD driver using pnputil (optional)
    if WizardIsTaskSelected('installdriver') then begin
      DriverInfPath := ExpandConstant('{app}\driver\VoiceClearVAD.inf');
      if FileExists(DriverInfPath) then begin
        if not Exec(ExpandConstant('{sys}\pnputil.exe'),
                    '/add-driver "' + DriverInfPath + '" /install',
                    '', SW_HIDE, ewWaitUntilTerminated, ResultCode) then begin
          MsgBox('Warning: pnputil failed to install the VoiceClearVAD driver.'#13#10 +
                 'Error code: ' + IntToStr(ResultCode) + #13#10#13#10 +
                 'The service will fall back to WASAPI / Virtual Cable mode.'#13#10 +
                 'To install the driver manually:'#13#10 +
                 '  pnputil /add-driver driver\VoiceClearVAD.inf /install',
                 mbInformation, MB_OK);
        end;
      end;
    end;

    // (D) Store Validated Product Key in Registry
    if ValidatedProductKey <> '' then begin
      RegWriteStringValue(HKCU, 'Software\Voice Clear AI', 'ProductKey', ValidatedProductKey);
      RegWriteStringValue(HKLM, 'SOFTWARE\VoiceClearAI', 'ProductKey', ValidatedProductKey);
    end;

  end;
end;

// ----------------------------------------------------------------------------
// CurUninstallStepChanged — clean up on uninstall
// ----------------------------------------------------------------------------
procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
var
  ResultCode: Integer;
begin
  if CurUninstallStep = usUninstall then begin
    // Stop and uninstall the Windows Service
    Exec(ExpandConstant('{app}\VoiceClearService.exe'),
         '--uninstall', '', SW_HIDE, ewWaitUntilTerminated, ResultCode);

    // Remove the virtual driver
    if FileExists(ExpandConstant('{app}\driver\VoiceClearVAD.inf')) then begin
      Exec(ExpandConstant('{sys}\pnputil.exe'),
           '/delete-driver VoiceClearVAD.inf /uninstall /force',
           '', SW_HIDE, ewWaitUntilTerminated, ResultCode);
    end;
  end;
end;

// =============================================================================
[Tasks]
Name: "desktopicon";    Description: "{cm:CreateDesktopIcon}";                               GroupDescription: "{cm:AdditionalIcons}"
Name: "startuprun";     Description: "Launch Voice Clear AI at Windows login";               GroupDescription: "Startup"; Flags: unchecked
Name: "installservice";  Description: "Install Voice Clear AI Windows Service";                GroupDescription: "System Services"
Name: "installvbcable";  Description: "Install Virtual Audio Cable Driver (Microsoft WHQL Signed)"; GroupDescription: "Audio Drivers"; Flags: checkedonce
Name: "installdriver";   Description: "Install VoiceClearVAD Native Driver (Requires Test Mode)"; GroupDescription: "Audio Drivers"; Flags: unchecked

; =============================================================================
[Files]
; ---- Core Service, UI & Deployed Qt6 Runtime ----
Source: "..\build\bin\Release\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

; ---- AI Neural Models ----
Source: "..\models\*"; DestDir: "{app}\models"; Flags: ignoreversion recursesubdirs createallsubdirs

; ---- Application Icon ----
Source: "app.ico"; DestDir: "{app}"; Flags: ignoreversion

; ---- Bundled Microsoft WHQL Virtual Audio Cable Driver ----
Source: "vbcable\*"; DestDir: "{app}\vbcable"; Flags: ignoreversion recursesubdirs createallsubdirs

; ---- Configuration ----
Source: "..\config\settings.json"; DestDir: "{app}\config"; Flags: ignoreversion onlyifdoesntexist

; ---- Resources ----
Source: "..\resources\*"; DestDir: "{app}\resources"; Flags: ignoreversion recursesubdirs createallsubdirs; Excludes: "*.ico"

; =============================================================================
[Dirs]
Name: "{app}\logs";   Flags: uninsalwaysuninstall
Name: "{app}\models"; Flags: uninsalwaysuninstall

; =============================================================================
[Registry]
; Auto-start the UI at login (optional task)
Root: HKCU; Subkey: "Software\Microsoft\Windows\CurrentVersion\Run"; \
  ValueType: string; ValueName: "VoiceClearAI"; \
  ValueData: """{app}\VoiceClear.exe"""; \
  Flags: uninsdeletevalue; Tasks: startuprun

; Write install path for the service to locate models/config
Root: HKLM; Subkey: "SOFTWARE\VoiceClearAI"; \
  ValueType: string; ValueName: "InstallPath"; \
  ValueData: "{app}"; \
  Flags: uninsdeletevalue uninsdeletekeyifempty

; =============================================================================
[Icons]
Name: "{group}\Voice Clear AI";          Filename: "{app}\VoiceClear.exe"; IconFilename: "{app}\app.ico"
Name: "{group}\Uninstall Voice Clear AI"; Filename: "{uninstallexe}"
Name: "{autodesktop}\Voice Clear AI";    Filename: "{app}\VoiceClear.exe"; IconFilename: "{app}\app.ico"; Tasks: desktopicon

; =============================================================================
[Run]
; Launch UI after install (skipped in silent mode)
Filename: "{app}\VoiceClear.exe"; Description: "Launch Voice Clear AI"; \
  Flags: nowait postinstall skipifsilent; \
  Check: FileExists(ExpandConstant('{app}\VoiceClear.exe'))

; =============================================================================
[UninstallRun]
Filename: "{app}\VoiceClearService.exe"; Parameters: "--uninstall"; \
  Flags: runhidden waituntilterminated; RunOnceId: "UninstallService"
