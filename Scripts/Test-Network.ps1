param(
    [string]$EngineRoot = 'C:\Program Files\Epic Games\UE_5.5',
    [ValidateSet('Win','Draw')][string]$Scenario = 'Win',
    [int]$Port = 17877,
    [switch]$Capture
)
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path $PSScriptRoot -Parent
$project = Join-Path $projectRoot 'CHAT.uproject'
$editor = Join-Path $EngineRoot 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
$logRoot = Join-Path $projectRoot ('Saved\NetworkTests\' + $Scenario + '-' + (Get-Date -Format 'yyyyMMdd-HHmmss'))
New-Item -ItemType Directory -Path $logRoot -Force | Out-Null
$processes = @()
function Launch-CHAT([string]$Name, [string[]]$Extra) {
    $log = Join-Path $logRoot ($Name + '.log')
    $arguments = @(('"' + $project + '"')) + $Extra + @('-unattended','-nosplash','-nosound','-nop4',('-abslog="' + $log + '"'))
    Start-Process -FilePath $editor -ArgumentList $arguments -WindowStyle Hidden -PassThru
}
try {
    $processes += Launch-CHAT 'Server' @('/Game/CHAT/Maps/L_Chat','-server','-nullrhi',('-port=' + $Port),'-CHATSmokeServer')
    $readyDeadline = (Get-Date).AddSeconds(90)
    do {
        Start-Sleep -Milliseconds 500
        $serverLog = Get-Content (Join-Path $logRoot 'Server.log') -Raw -ErrorAction SilentlyContinue
        if ($processes[0].HasExited) { throw 'Server exited before listening.' }
    } until ($serverLog -match 'IpNetDriver listening on port' -or (Get-Date) -gt $readyDeadline)
    if ($serverLog -notmatch 'IpNetDriver listening on port') { throw 'Server startup timed out.' }
    Write-Output ('Server ready: ' + $logRoot)
    foreach ($role in @('A','B')) {
        $clientArgs = @(('127.0.0.1:' + $Port),'-game',('-CHATSmokeRole=' + $role),('-CHATSmokeScenario=' + $Scenario))
        if ($Capture -and $role -eq 'A') {
            $clientArgs += @('-RenderOffscreen','-windowed','-ResX=1100','-ResY=760',('-CHATCapture="' + (Join-Path $logRoot 'Result.png') + '"'))
        } else { $clientArgs += '-nullrhi' }
        $processes += Launch-CHAT $role $clientArgs
    }
    $deadline = (Get-Date).AddSeconds(120)
    do {
        Start-Sleep -Seconds 1
        $a = Get-Content (Join-Path $logRoot 'A.log') -Raw -ErrorAction SilentlyContinue
        $b = Get-Content (Join-Path $logRoot 'B.log') -Raw -ErrorAction SilentlyContinue
        if (($a + $b) -match 'CHAT_SMOKE_FAIL') { throw 'Network assertions failed; inspect logs.' }
        if ($processes.Where({ $_.HasExited }).Count -gt 0) { throw 'A test process exited unexpectedly.' }
    } until (($a -match 'CHAT_SMOKE_PASS' -and $b -match 'CHAT_SMOKE_PASS') -or (Get-Date) -gt $deadline)
    if ($a -notmatch 'CHAT_SMOKE_PASS' -or $b -notmatch 'CHAT_SMOKE_PASS') { throw 'Network test timed out.' }
    Write-Output ('PASS: ' + $Scenario + ' - both clients verified RPCs, attempts, result and reset.')
    Get-ChildItem $logRoot -Filter '*.log' | Select-String 'CHAT_SMOKE_' | ForEach-Object { $_.Line }
} finally {
    foreach ($process in $processes) {
        if (!$process.HasExited) { Stop-Process -Id $process.Id -ErrorAction SilentlyContinue }
    }
}
