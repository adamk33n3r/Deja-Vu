Set-Location $PSScriptRoot
$ErrorActionPreference = 'stop'

function IsInteractive {
    # not including `-NonInteractive` since it apparently does nothing
    # "Does not present an interactive prompt to the user" - no, it does present!
    $non_interactive = '-command', '-c', '-encodedcommand', '-e', '-ec', '-file', '-f', '-NonInteractive'

    # alternatively `$non_interactive [-contains|-eq] $PSItem`
    -not ([Environment]::GetCommandLineArgs() | Where-Object -FilterScript {$PSItem -in $non_interactive})
}

try {

    $matchInfo = Select-String -Path .\Version.h -Pattern 'VERSION_.* (.+)'
    $phaseDefine = $matchInfo.Matches.Groups[9].Value
    $phaseMatchInfo = Select-String -Path .\Version.h -Pattern ($phaseDefine + ' (.+)')
    $versionString = $matchInfo.Matches.Groups[1].Value + '.' + $matchInfo.Matches.Groups[3].Value + '.' + $matchInfo.Matches.Groups[5].Value + '.' + $matchInfo.Matches.Groups[7].Value + $phaseMatchInfo.Matches.Groups[1].Value
    Write-Output "Creating build version: $versionString"

    $buildDir = (Get-Location).ToString() + '\zipDir'
    New-Item -Force -ItemType Directory -Path .\builds | Out-Null
    $zipFile = ".\builds\DejaVu - $versionString.zip"

    if (Test-Path $zipFile -PathType Leaf) {
        if (-not (IsInteractive)) {
            exit
        }

        [System.Reflection.Assembly]::LoadWithPartialName("System.Windows.Forms")
        $msgBoxInput = [System.Windows.Forms.MessageBox]::Show('Release zip already exists. Do you want to overwrite?', 'File exists', 'YesNo', 'Error')

        switch ($msgBoxInput) {
            'Yes' {
                Remove-Item $zipFile
            }

            'No' {
                exit
            }
        }
    }

    Remove-Item $buildDir -Recurse
    New-Item -ItemType Directory -Path $buildDir\plugins | Out-Null
    Copy-Item .\x64\Release\DejaVuPlugin.dll -Destination $buildDir\plugins
    New-Item -ItemType Directory -Path $buildDir\source | Out-Null
    Copy-Item .\vendor -Destination $buildDir\source -Recurse
    Copy-Item .\bakkesmodsdk -Destination $buildDir\source -Recurse
    $fileList = (
        '.\Canvas.cpp',
        '.\Canvas.h',
        '.\CVar2WayBinding.h',
        '.\Deja Vu.filters',
        '.\Deja Vu.sln',
        '.\Deja Vu.vcxproj',
        '.\Deja Vu.vcxproj.user',
        '.\DejaVu.cpp',
        '.\DejaVu.h',
        '.\DejaVuGUI.cpp',
        '.\pch.cpp',
        '.\pch.h',
        '.\SettingsFileGenerator.cpp',
        '.\LICENSE',
        '.\RuleSet.ruleset',
        '.\Version.h'
    )
    foreach ($file in $fileList) {
        Copy-Item $file -Destination $buildDir\source
    }

    Add-Type -Assembly System.IO.Compression.FileSystem
    [System.IO.Compression.ZipFile]::CreateFromDirectory(
        $buildDir,
        $zipFile,
        [System.IO.Compression.CompressionLevel]::Optimal,
        $false
    )
} catch {
    Write-Error($_.Exception.Message)
}

if (IsInteractive) {
    Pause
}