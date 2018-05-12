@echo off
fxc -nologo /Od /Zi /E vs_main /T vs"_5_0" /Fo %2.vo %1.hlsl
fxc -nologo /Od /Zi /E ps_main /T ps"_5_0" /Fo %2.po %1.hlsl
