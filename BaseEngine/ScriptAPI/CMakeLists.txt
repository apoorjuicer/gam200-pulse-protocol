# ScriptAPI - C++/CLI bridge between C++ and C#
project(ScriptAPI LANGUAGES CXX)

# Create library with placeholder
add_library(ScriptAPI SHARED
    TestAPI.h
    TestAPI.cpp
)

# VSC properties setting for ScriptAPI (Cos its C++/CLI, its a lil complicated so we automatically export it)
set_target_properties(ScriptAPI PROPERTIES
    WINDOWS_EXPORT_ALL_SYMBOLS ON
)

# Enable C++/CLI (Common Language Runtime)
set_target_properties(ScriptAPI PROPERTIES
    COMMON_LANGUAGE_RUNTIME ""
    VS_GLOBAL_CLRSupport "true"
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

# Include directories
target_include_directories(ScriptAPI PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/extern/dotnet/include
    ${CMAKE_SOURCE_DIR}/MyEngine  # ← Can access MyEngine headers
)

# Output to same directory as MyEngine
set_target_properties(ScriptAPI PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/MyEngine/$<CONFIG>"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/MyEngine/$<CONFIG>"
)
    
# Set folder organization for Visual Studio
set_property(TARGET ScriptAPI PROPERTY FOLDER "")

# Enables multi-processor parallel compilation in Visual Studio
if(MSVC)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif()