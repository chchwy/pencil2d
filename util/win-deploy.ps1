Param(
  [string]$platform = "x64",  # x64/x86
  [string]$branch = "master", # branch names: master, release
  [string]$upload = "no"      # yes/no
  [string]$compiler = ""      # msvc/mingw  
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

cd $PSScriptRoot
cd ../build

echo ">>> Current working directory:"
pwd # print the current working directory

mkdir pencil2d
cp './app/release/pencil2d.exe' './pencil2d/pencil2d.exe'
mkdir './pencil2d/plugins'

echo ">>> Downloading ffmpeg: $ffmpegUrl"
[string]$ffmpegZipFile = "ffmpeg-$arch.zip"
[string]$ffmpegUrl = "https://github.com/pencil2d/pencil2d-deps/releases/download/ffmpge-v4.1.1/$ffmpegZipFile"
wget -Uri $ffmpegUrl -OutFile "$ffmpegZipFile" -ErrorAction Stop
Expand-Archive -Path "$ffmpegZipFile" -DestinationPath "./pencil2d/plugins" -ErrorAction Stop

echo ">>> Clean up ffmpeg"
Remove-Item -Path "./$ffmpegZipFile"
Remove-Item -Path "./pencil2d/*.pdb"
Remove-Item -Path "./pencil2d/*.ilk"

echo ">>> Deploying Qt libraries"
&windeployqt @("pencil2d/pencil2d.exe")

echo ">>> Copy OpenSSL DLLs"
cp $libcrypto "./pencil2d"
cp $libssl    "./pencil2d"

echo ">>> Zipping pencil2d folder"
Compress-Archive -Path "./pencil2d" -DestinationPath "./Pencil2D.zip"

$today = Get-Date -Format "yyyy-MM-dd"
$buildNumber = $env:APPVEYOR_BUILD_NUMBER
$zipFileName = "pencil2d-$arch-$buildNumber-$compiler.zip"

echo ">>> Zip filename: $zipFileName"
Rename-Item -Path "./Pencil2D.zip" -NewName $zipFileName

echo ">>> Zip ok?"
Test-Path $zipFileName

if ($upload -ne "yes") {
  echo ">>> Done. No need to upload."
  exit 0
}

echo ">>> Uploading to Dropbox..."
$python3 = "C:\Python36\python.exe"

$fullPath = "$zipFileName"
& $python3 @("../util/fileuploader.py", $fullPath)

echo ">>> Done!"
