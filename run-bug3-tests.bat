@echo off
set PATH=%PATH%;C:\Qt\6.5.3\msvc2019_64\bin
cd /d %~dp0tests\debug
echo Running Bug #3 Tests...
tests.exe "LayerBitmap::presave - Bug #3 Error Handling Tests"
