set(CMAKE_CSharp_FLAGS "/platform:anycpu")
set(CMAKE_CONFIGURATION_TYPES "Debug;Release;RelWithDebInfo;MinSizeRel" CACHE STRING "" FORCE)

# Use manual .csproj instead of CMake-generated C# project
include_external_msproject(
    ManagedScripts 
    "${CMAKE_CURRENT_SOURCE_DIR}/ManagedScripts.csproj"
    TYPE FAE04EC0-301F-11D3-BF4B-00C04F79EFBC
    #PLATFORM x64
)

# Tell CMake that ManagedScripts depends on ScriptAPI (In the guide, manage needs script)
add_dependencies(ManagedScripts ScriptAPI)

# Force it to be included in ALL build configurations (cos its stubbon af :D )
set_target_properties(ManagedScripts PROPERTIES
    EXCLUDE_FROM_DEFAULT_BUILD FALSE
    EXCLUDE_FROM_DEFAULT_BUILD_DEBUG FALSE
    EXCLUDE_FROM_DEFAULT_BUILD_RELEASE FALSE
    EXCLUDE_FROM_DEFAULT_BUILD_RELWITHDEBINFO FALSE
    EXCLUDE_FROM_DEFAULT_BUILD_MINSIZEREL FALSE
)

# Map CMake configurations to .csproj configurations
set_target_properties(ManagedScripts PROPERTIES
    MAP_IMPORTED_CONFIG_RELEASE Release
    MAP_IMPORTED_CONFIG_DEBUG Debug
    MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release
    MAP_IMPORTED_CONFIG_MINSIZEREL Release
)

# Set folder organization for Visual Studio
set_property(TARGET ManagedScripts PROPERTY FOLDER "")