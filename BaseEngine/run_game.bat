@echo off
REM === Build game configuration (MyGame.exe, engine WITHOUT editor hooks) ===

set BUILD_DIR=build

REM Create build folder if needed
if not exist "%BUILD_DIR%" (
    mkdir "%BUILD_DIR%"
)

pushd "%BUILD_DIR%"

REM Configure CMake for GAME build (no editor)
cmake .. ^
    -G "Visual Studio 17 2022" ^
    -A x64 ^
    -DENABLE_EDITOR=OFF ^
    -DCMAKE_POLICY_VERSION_MINIMUM=3.5

REM Build Debug game (you can add --target MyGame if you only want that)
cmake --build . --config Debug

popd

REM (Docs copy is optional for game; add if you want)
REM cmake -E copy_if_different "%~dp0MyEngine\Bloop_ReadMe.txt" "%~dp0build\Bloop_ReadMe.txt"
REM cmake -E copy_if_different "%~dp0MyEngine\projectGuide.txt" "%~dp0build\projectGuide.txt"

echo.
echo ============================================
echo Game build done. Open:
echo   %~dp0build_game\MyEngine.sln
echo and run MyGame (Debug).
echo ============================================
pause
