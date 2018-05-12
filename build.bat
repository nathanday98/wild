@echo off
set compiler_options=-MT -D_ITERATOR_DEBUG_LEVEL=0 -D_CRT_SECURE_NO_WARNINGS -W4 -FC -WX -Oi -Od -Gm- -GR- -Z7 -nologo -wd4312 -wd4806 -wd4701 -wd4505 -wd4201 -EHsc -wd4100 -wd4127 -wd4189 -wd4244 -Fo:..\temp\ -I..\deps\include -I..\src
set linker_options=-incremental:no -opt:ref -LIBPATH:../deps/lib/x64/windows/ 

if not exist build mkdir build
if not exist temp mkdir temp

pushd build
del *.pdb > NUL 2> NUL

xcopy ..\deps\bin\x64\*.dll . /Y /S /E /Q /D >NUL

If Not Exist imgui.lib  cl -Fe:imgui.dll  ../src/imgui/imgui.cpp ../src/imgui/imgui_demo.cpp ../src/imgui/imgui_draw.cpp -LD -nologo

cl %compiler_options% ../src/game/game.cpp -DPP_EDITOR -LD /link %linker_options% sdl2.lib sdl2main.lib imgui.lib /DLL -PDB:game_%random%.pdb /EXPORT:gameInit /EXPORT:gameUpdate /EXPORT:gameRender 

cl %compiler_options% -Fe:wild.exe -MP ../src/main.cpp ../src/core/win32/platform.cpp -Fm:wild.map /link %linker_options% imgui.lib dxgi.lib d3d11.lib user32.lib sdl2.lib sdl2main.lib -SUBSYSTEM:CONSOLE

popd


pushd data\shaders
call ..\..\compile_shader.bat ..\..\src\shaders\geometry geometry
call ..\..\compile_shader.bat ..\..\src\shaders\imgui imgui
call ..\..\compile_shader.bat ..\..\src\shaders\lights lights
call ..\..\compile_shader.bat ..\..\src\shaders\textured textured
popd