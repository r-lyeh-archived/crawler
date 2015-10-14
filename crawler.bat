@echo off
cd /D "%~dp0"

if /i "%1" == "clean" (
    @if exist .prj   rd /q /s .prj  1> nul 2> nul
    @if exist .obj   rd /q /s .obj  1> nul 2> nul
    @if exist .data  rd /q /s .data 1> nul 2> nul
    @rem premake5 clean
    @rem git checkout bin/*

    for /D %%i in (apps\*) do (
    if exist "%%i\*.ilk" del "%%i\*.ilk"
    if exist "%%i\*.exe" del "%%i\*.exe"
    if exist "%%i\*.pdb" del "%%i\*.pdb"
    if exist "%%i\*.suo" del "%%i\*.suo"
    if exist "%%i\*.dll" del "%%i\*.dll"
    if exist "%%i\*.map" del "%%i\*.map"
    )

    for /D %%i in (tutorials\*) do (
    if exist "%%i\*.ilk" del "%%i\*.ilk"
    if exist "%%i\*.exe" del "%%i\*.exe"
    if exist "%%i\*.pdb" del "%%i\*.pdb"
    if exist "%%i\*.suo" del "%%i\*.suo"
    if exist "%%i\*.dll" del "%%i\*.dll"
    if exist "%%i\*.map" del "%%i\*.map"
    )

    echo [ OK ] Clean up
    exit /b
)

echo [ OK ] Generating licenses...
premake5 --gen-licenses > nul
echo [ OK ] Generating projects...
premake5 vs2013 1> nul 2> nul
echo [ OK ] Running solution...
start .prj\crawler.sln
