[CmdletBinding()]
param(
    [ValidateSet('Release', 'Debug')]
    [string]$Configuration = 'Release',
    [string]$QtRoot = '',
    [switch]$Clean
)

$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
$sourceDir = Join-Path $projectRoot 'apps\overlay_qt'
$buildDir = Join-Path $projectRoot "build\overlay-windows-$($Configuration.ToLowerInvariant())"
$packageDir = Join-Path $projectRoot 'dist\AntiCarsickOverlay'

if ($Clean -and (Test-Path -LiteralPath $buildDir)) {
    $resolvedBuild = (Resolve-Path -LiteralPath $buildDir).Path
    $resolvedRoot = (Resolve-Path -LiteralPath $projectRoot).Path
    if (-not $resolvedBuild.StartsWith($resolvedRoot, [System.StringComparison]::OrdinalIgnoreCase)) {
        throw "Refusing to clean a directory outside the project: $resolvedBuild"
    }
    Remove-Item -LiteralPath $resolvedBuild -Recurse -Force
}

$cmake = Get-Command cmake -ErrorAction SilentlyContinue
if (-not $cmake) {
    $pythonScripts = Join-Path $env:APPDATA 'Python\Python314\Scripts'
    $cmakeCandidate = Join-Path $pythonScripts 'cmake.exe'
    if (Test-Path -LiteralPath $cmakeCandidate) {
        $cmake = Get-Item -LiteralPath $cmakeCandidate
        $env:PATH = "$pythonScripts;$env:PATH"
    }
}
if (-not $cmake) {
    throw 'CMake was not found. Install CMake and enable "Add CMake to PATH".'
}

if (-not (Get-Command cl.exe -ErrorAction SilentlyContinue)) {
    $vsDevCmdCandidates = @(
        'C:\BuildTools\Common7\Tools\VsDevCmd.bat',
        'C:\Program Files\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\VsDevCmd.bat',
        'C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat'
    )
    $vsDevCmd = $vsDevCmdCandidates | Where-Object { Test-Path -LiteralPath $_ } | Select-Object -First 1
    if (-not $vsDevCmd) {
        throw 'MSVC was not found. Install Visual Studio 2022 Desktop development with C++.'
    }

    $environmentLines = & cmd.exe /s /c "`"$vsDevCmd`" -arch=x64 -host_arch=x64 && set"
    $importedNames = @{}
    foreach ($line in $environmentLines) {
        if ($line -match '^([^=]+)=(.*)$') {
            $name = $matches[1]
            if (-not $importedNames.ContainsKey($name)) {
                Set-Item -Path "Env:$name" -Value $matches[2]
                $importedNames[$name] = $true
            }
        }
    }
}

$qtCmake = Get-Command qt-cmake -ErrorAction SilentlyContinue
if ($QtRoot) {
    $candidate = Join-Path $QtRoot 'bin\qt-cmake.bat'
    if (-not (Test-Path -LiteralPath $candidate)) {
        throw "qt-cmake.bat was not found under $QtRoot\bin"
    }
    $qtCmakePath = $candidate
} elseif ($qtCmake) {
    $qtCmakePath = $qtCmake.Source
} else {
    throw 'qt-cmake was not found. Open a Qt command prompt or pass -QtRoot C:\Qt\6.x.x\msvc2022_64.'
}

New-Item -ItemType Directory -Path $buildDir -Force | Out-Null
$configureArgs = @('-S', $sourceDir, '-B', $buildDir, "-DCMAKE_BUILD_TYPE=$Configuration")
if (Get-Command ninja.exe -ErrorAction SilentlyContinue) {
    $configureArgs += @('-G', 'Ninja')
}
& $qtCmakePath @configureArgs
if ($LASTEXITCODE -ne 0) { throw 'CMake configuration failed.' }

& $cmake.Source --build $buildDir --config $Configuration --parallel
if ($LASTEXITCODE -ne 0) { throw 'Build failed.' }

if (Test-Path -LiteralPath $packageDir) {
    $resolvedPackage = (Resolve-Path -LiteralPath $packageDir).Path
    $resolvedRoot = (Resolve-Path -LiteralPath $projectRoot).Path
    if (-not $resolvedPackage.StartsWith($resolvedRoot, [System.StringComparison]::OrdinalIgnoreCase)) {
        throw "Refusing to replace a directory outside the project: $resolvedPackage"
    }
    Remove-Item -LiteralPath $packageDir -Recurse -Force
}
New-Item -ItemType Directory -Path $packageDir -Force | Out-Null

$exeCandidates = @(
    (Join-Path $buildDir "$Configuration\MotionOverlay.exe"),
    (Join-Path $buildDir 'MotionOverlay.exe')
)
$exe = $exeCandidates | Where-Object { Test-Path -LiteralPath $_ } | Select-Object -First 1
if (-not $exe) { throw 'MotionOverlay.exe was not produced.' }

Copy-Item -LiteralPath $exe -Destination $packageDir
$windeployqt = Join-Path (Split-Path -Parent $qtCmakePath) 'windeployqt.exe'
if (-not (Test-Path -LiteralPath $windeployqt)) {
    $deployCommand = Get-Command windeployqt -ErrorAction SilentlyContinue
    if (-not $deployCommand) { throw 'windeployqt was not found.' }
    $windeployqt = $deployCommand.Source
}

$packagedExe = Join-Path $packageDir 'MotionOverlay.exe'
& $windeployqt --release --no-translations $packagedExe
if ($LASTEXITCODE -ne 0) { throw 'Qt runtime deployment failed.' }

Write-Host "Windows package ready: $packageDir" -ForegroundColor Green
