@echo off

cl.exe /nologo /EHsc /O2 tinyspacer.cpp user32.lib shell32.lib gdi32.lib /link /SUBSYSTEM:WINDOWS