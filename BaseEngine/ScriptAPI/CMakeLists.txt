# ScriptAPI - C++/CLI bridge between C++ and C#
project(ScriptAPI LANGUAGES CXX)

# Create library with placeholder
add_library(ScriptAPI SHARED
    TestAPI.h
    TestAPI.cpp
)

# Enable C++/CLI (Common Language Runtime)
set_target_properties(ScriptAPI PROPERTIES
    COMMON_LANGUAGE_RUNTIME ""
    VS_GLOBAL_CLRSupport "true"
    CXX_STANDARD 17
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