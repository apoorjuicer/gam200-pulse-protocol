if %errorlevel% neq 0 goto :cmEnd
:cmEnd
endlocal & call :cmErrorLevel %errorlevel% & goto :cmDone
:cmErrorLevel
exit /b %1
:cmDone
if %errorlevel% neq 0 goto :VCEnd
setlocal
"C:\Program Files\CMake\bin\cmake.exe" -E copy_if_different C:/Users/eugen/sandbox/Y2T1/GAM/csd2401f25_team_bloop/BaseEngine/MyEngine/FMOD_API/api/studio/lib/x64/fmodstudioL.dll C:/Users/eugen/sandbox/Y2T1/GAM/csd2401f25_team_bloop/BaseEngine/build/MyEngine/Debug
if %errorlevel% neq 0 goto :cmEnd
:cmEnd
endlocal & call :cmErrorLevel %errorlevel% & goto :cmDone
:cmErrorLevel
exit /b %1
:cmDone
if %errorlevel% neq 0 goto :VCEnd
setlocal
"C:\Program Files\CMake\bin\cmake.exe" -E copy_directory C:/Users/eugen/sandbox/Y2T1/GAM/csd2401f25_team_bloop/BaseEngine/MyEngine/JSON C:/Users/eugen/sandbox/Y2T1/GAM/csd2401f25_team_bloop/BaseEngine/build/MyEngine/JSON
if %errorlevel% neq 0 goto :cmEnd
:cmEnd
endlocal & call :cmErrorLevel %errorlevel% & goto :cmDone
:cmErrorLevel
exit /b %1
:cmDone
if %errorlevel% neq 0 goto :VCEnd
setlocal
"C:\Program Files\CMake\bin\cmake.exe" -E copy_directory C:/Users/eugen/sandbox/Y2T1/GAM/csd2401f25_team_bloop/BaseEngine/MyEngine/Assets C:/Users/eugen/sandbox/Y2T1/GAM/csd2401f25_team_bloop/BaseEngine/build/MyEngine/Assets
if %errorlevel% neq 0 goto :cmEnd
:cmEnd
endlocal & call :cmErrorLevel %errorlevel% & goto :cmDone
:cmErrorLevel
exit /b %1
:cmDone
if %errorlevel% neq 0 goto :VCEnd
setlocal
"C:\Program Files\CMake\bin\cmake.exe" -E copy_directory C:/Users/eugen/sandbox/Y2T1/GAM/csd2401f25_team_bloop/BaseEngine/extern/dotnet/bin C:/Users/eugen/sandbox/Y2T1/GAM/csd2401f25_team_bloop/BaseEngine/build/MyEngine/Debug
if %errorlevel% neq 0 goto :cmEnd
:cmEnd
endlocal & call :cmErrorLevel %errorlevel% & goto :cmDone
:cmErrorLevel
exit /b %1
:cmDone
if %errorlevel% neq 0 goto :VCEnd
setlocal
"C:\Program Files\CMake\bin\cmake.exe" -E copy_directory C:/Users/eugen/sandbox/Y2T1/GAM/csd2401f25_team_bloop/BaseEngine/build/MyEngine/Debug/net8.0 C:/Users/eugen/sandbox/Y2T1/GAM/csd2401f25_team_bloop/BaseEngine/build/MyEngine/Debug
if %errorlevel% neq 0 goto :cmEnd
"C:\Program Files\CMake\bin\cmake.exe" -E remove_directory C:/Users/eugen/sandbox/Y2T1/GAM/csd2401f25_team_bloop/BaseEngine/build/MyEngine/Debug/net8.0
if %errorlevel% neq 0 goto :cmEnd
:cmEnd
endlocal & call :cmErrorLevel %errorlevel% & goto :cmDone
:cmErrorLevel
exit /b %1
:cmDone
if %errorlevel% neq 0 goto :VCEnd