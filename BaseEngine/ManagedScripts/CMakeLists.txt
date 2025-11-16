# Use manual .csproj instead of CMake-generated C# project
include_external_msproject(
    ManagedScripts 
    "${CMAKE_CURRENT_SOURCE_DIR}/ManagedScripts.csproj"
    TYPE FAE04EC0-301F-11D3-BF4B-00C04F79EFBC
)

# Set folder organization for Visual Studio
set_property(TARGET ManagedScripts PROPERTY FOLDER "")