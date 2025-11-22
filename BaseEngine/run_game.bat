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

echo.
echo ============================================
echo Game build done. Open:
echo   %~dp0build_game\MyEngine.sln
echo and run MyGame (Debug).
echo ============================================
pause
