@echo off
mkdir "%~dp0..\build"
pushd "%~dp0..\build"
cl -Zi -Fa -I ..\code ..\code\main.cpp
.\main.exe "..\samples\build\entry.obj"
echo Exit code: %errorlevel%
popd
