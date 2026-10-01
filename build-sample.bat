@echo off
mkdir "%~dp0.\samples\build"
pushd "%~dp0.\samples\build"
cl -Zi -I ..\code  ..\code\addition.cpp ..\code\entry.cpp
popd
