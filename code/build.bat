@echo off
mkdir ..\build
pushd ..\build

REM -Zi is for debugging mode
cl -Zi ..\code\win32_handmade.cpp user32.lib gdi32.lib
popd