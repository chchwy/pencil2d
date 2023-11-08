Param(
  [string]$platform = "x64",  # x64/x86
  [string]$branch = "master",   # branch names: master, release
  [string]$upload = "no"        # yes/no
)

echo ">>> Upload?", $upload
echo ">>> Branch:", $branch
echo ">>> Platform:", $platform

$arch = switch ($platform) {
  "x86" {"win32"; break}
  "x64" {"win64"; break}
  default {"Unknown"; break}
}

$libcrypto = switch ($platform) {
  "x86" {"C:\OpenSSL-v111-Win32\bin\libcrypto-1_1.dll"; break}
  "x64" {"C:\OpenSSL-v111-Win64\bin\libcrypto-1_1-x64.dll"; break}
  default {""; break}
}

$libssl = switch ($platform) {
  "x86" {"C:\OpenSSL-v111-Win32\bin\libssl-1_1.dll"; break}
  "x64" {"C:\OpenSSL-v111-Win64\bin\libssl-1_1-x64.dll"; break}
  default {""; break}
}

[string]$ffmpegFileName = "ffmpeg-$arch.zip"
[string]$ffmpegUrl = "https://github.com/pencil2d/pencil2d-deps/releases/download/ffmpge-v4.1.1/$ffmpegFileName"

echo $PSScriptRoot
cd $PSScriptRoot
cd ..
echo "Find pencil2d.exe"
Get-ChildItem -Include *.exe -File -Recurse

cd build

echo ">>> Current working directory:"
pwd # print the current working directory

mkdir pencil2d
cp ./app/release/pencil2d.exe ./pencil2d/pencil2d.exe

New-Item -ItemType 'directory' -Path './pencil2d/plugins' -ErrorAction Continue

echo ">>> Downloading ffmpeg: $ffmpegUrl"
 
wget -Uri $ffmpegUrl -OutFile "$ffmpegFileName" -ErrorAction Stop
Expand-Archive -Path "$ffmpegFileName" -DestinationPath "./pencil2d/plugins" -ErrorAction Stop

echo ">>> Clean up ffmpeg"

Remove-Item -Path "./$ffmpegFileName"
Remove-Item -Path "./pencil2d/*.pdb"
Remove-Item -Path "./pencil2d/*.ilk"

echo ">>> Deploying Qt libraries"

& "windeployqt" @("pencil2d/pencil2d.exe")

echo ">>> Copy OpenSSL DLLs"
Copy-Item $libcrypto -Destination "./pencil2d"
Copy-Item $libssl -Destination "./pencil2d"

echo ">>> Zipping pencil2d folder"

Compress-Archive -Path "./pencil2d" -DestinationPath "./Pencil2D.zip"

$today = Get-Date -Format "yyyy-MM-dd"
$zipFileName = "pencil2d-$arch-$today.zip"

echo ">>> Zip filename: $zipFileName"
Rename-Item -Path "./Pencil2D.zip" -NewName $zipFileName

echo ">>> Zip ok?"
Test-Path $zipFileName

pwd
ls ./pencil2d

cd $PSScriptRoot

if ($upload -ne "yes") {
  echo ">>> Done. No need to upload binaries."
  exit 0
}

echo ">>> Upload to Google drive"

$python3 = if (Test-Path env:PYTHON) { "$env:PYTHON\python.exe" } else { "python.exe" }

$GDriveFolderId = switch($platform) {
  "x86" {$env:WIN32_NIGHTLY_PARENT; break}
  "x64" {$env:WIN64_NIGHTLY_PARENT; break}
}

$fullPath = Convert-Path "..\build\$zipFileName"

& $python3 @("nightly-build-upload.py", $GDriveFolderId, $fullPath)

echo ">>> Done!"
