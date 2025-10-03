@echo off

REM CHECK IF 'build' FOLDER EXISTS; CREATE IF IT DOES NOT
if not exist build (
    mkdir build
)

REM BUILDING DEBUG
pushd build
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_POLICY_VERSION_MINIMUM=3.5
cmake --build . --config Debug
popd

REM ---- COPY DOCS
cmake -E copy_if_different "%~dp0MyEngine\Bloop_ReadMe.txt" "%~dp0build\Bloop_ReadMe.txt"
cmake -E copy_if_different "%~dp0MyEngine\projectGuide.txt" "%~dp0build\projectGuide.txt"

REM BUILDING RELEASE
if not exist build (
    mkdir build
)

pushd build
cmake ..
cmake --build . --config Release
popd
