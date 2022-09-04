@echo off
IF NOT EXIST ..\build mkdir ..\build
pushd  ..\build

cl /Femain.exe /Zi ..\code\gui.cpp User32.lib Gdi32.lib  winmm.lib dwmapi.lib xinput.lib

popd
