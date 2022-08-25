@echo off
cl /DEBUG:FULL /Femain.exe /Zi gui.cpp User32.lib Gdi32.lib  winmm.lib dwmapi.lib  
