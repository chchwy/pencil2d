set PATH=%PATH%;C:\Qt\6.5.3\msvc2019_64\bin
call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvarsall.bat" x64

cd %~dp0
qmake ..\pencil2d.pro -r -spec win32-msvc "CONFIG+=release" "CONFIG+=x64"
msbuild pencil2d.sln /p:Configuration=Release /p:Platform=x64

