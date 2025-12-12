@echo off
REM === Build editor configuration (MyEditor.exe, engine with editor hooks) ===

set BUILD_DIR=build

REM Create build folder if needed
if not exist "%BUILD_DIR%" (
    mkdir "%BUILD_DIR%"
)

pushd "%BUILD_DIR%"

REM Configure CMake for EDITOR build
cmake .. ^
    -G "Visual Studio 17 2022" ^
    -A x64 ^
    -DENABLE_EDITOR=ON ^
    -DCMAKE_POLICY_VERSION_MINIMUM=3.5

REM Build Debug editor (you can add --target MyEditor if you only want that)
cmake --build . --config Debug

echo Building Release configuration...
cmake --build . --config Release

popd

echo.
echo ============================================
echo Editor build done. Open:
echo   %~dp0build_editor\MyEngine.sln
echo and run MyEditor (Debug).
echo ============================================
pause
