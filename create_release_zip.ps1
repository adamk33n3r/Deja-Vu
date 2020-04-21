Set-Location C:\Users\adamg\GitHub\Deja-Vu

$matchInfo = Select-String -Path .\DejaVu.cpp -Pattern 'BAKKESMOD_PLUGIN\(DejaVu, "Deja Vu", "(.*)",'
$versionString = $matchInfo.Matches.Groups[1].Value
Write-Output $versionString

$buildDir = 'C:\Users\adamg\GitHub\Deja-Vu\zipDir'
$zipFile = "$buildDir\..\DejaVu - $versionString.zip"

if (Test-Path $zipFile -PathType Leaf)
{
    [System.Reflection.Assembly]::LoadWithPartialName("System.Windows.Forms")
    $msgBoxInput = [System.Windows.Forms.MessageBox]::Show('Release zip already exists. Do you want to overwrite?', 'File exists', 'YesNo', 'Error')

    switch  ($msgBoxInput) {
        'Yes' {
            Remove-Item $zipFile
        }

        'No' {
            exit
        }
  }
}

Remove-Item $buildDir -Recurse
New-Item -ItemType Directory -Path $buildDir\plugins\settings | Out-Null
Copy-Item dejavu.set -Destination $buildDir\plugins\settings
New-Item -ItemType Directory -Path $buildDir\source | Out-Null
Copy-Item .\vendor -Destination $buildDir\source -Recurse
Copy-Item .\bakkesmodsdk -Destination $buildDir\source -Recurse
$fileList = (
    '.\Canvas.cpp',
    '.\Canvas.h',
    '.\Deja Vu.filters',
    '.\Deja Vu.sln',
    '.\Deja Vu.vcxproj',
    '.\Deja Vu.vcxproj.user',
    '.\DejaVu.cpp',
    '.\DejaVu.h',
    '.\DejaVuGUI.cpp',
    '.\LICENSE',
    '.\RuleSet.ruleset'
)
foreach ($file in $fileList)
{
    Copy-Item $file -Destination $buildDir\source
}

Add-Type -Assembly System.IO.Compression.FileSystem
[System.IO.Compression.ZipFile]::CreateFromDirectory(
    $buildDir,
    $zipFile,
    [System.IO.Compression.CompressionLevel]::Optimal,
    $false
)

Pause