param([string]$EngineRoot = 'C:\Program Files\Epic Games\UE_5.5', [int]$Port = 17777)
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path $PSScriptRoot -Parent
$project = Join-Path $projectRoot 'CHAT.uproject'
$editor = Join-Path $EngineRoot 'Engine\Binaries\Win64\UnrealEditor.exe'
if (!(Test-Path (Join-Path $projectRoot 'Binaries\Win64\UnrealEditor-CHAT.dll'))) {
    throw 'Open CHAT.uproject and compile the project first.'
}
# Two visible game windows are the requested interactive play session.
$hostLog = Join-Path $projectRoot ('Saved\LocalHost-' + (Get-Date -Format 'yyyyMMdd-HHmmss') + '.log')
$hostProcess = Start-Process $editor -ArgumentList @(('"' + $project + '"'),('/Game/CHAT/Maps/L_Chat?listen'),'-game',('-port=' + $Port),'-windowed','-ResX=960','-ResY=720','-WinX=20','-WinY=40',('-abslog="' + $hostLog + '"')) -PassThru
$deadline = (Get-Date).AddSeconds(90)
do {
    Start-Sleep -Milliseconds 500
    $logText = Get-Content $hostLog -Raw -ErrorAction SilentlyContinue
    if ($hostProcess.HasExited) { throw 'Host exited. Check the log in Saved.' }
} until ($logText -match 'IpNetDriver listening on port' -or (Get-Date) -gt $deadline)
if ($logText -notmatch 'IpNetDriver listening on port') { throw 'Host startup timed out. Check the host window.' }
Start-Process $editor -ArgumentList @(('"' + $project + '"'),('127.0.0.1:' + $Port),'-game','-windowed','-ResX=960','-ResY=720','-WinX=980','-WinY=40')
