# ScriptAPI - C++/CLI bridge between C++ and C#
project(ScriptAPI LANGUAGES CXX)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

# Auto-collect sources (keeps Solution Explorer in sync)
file(GLOB_RECURSE SCRIPTAPI_SRC CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_LIST_DIR}/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/*.h"
)


# Create library with placeholder
add_library(ScriptAPI SHARED ${SCRIPTAPI_SRC})

# add to build depen
#add_dependencies(ScriptAPI MyEngine)

target_link_libraries(ScriptAPI
    PRIVATE
        MyEngine
)

# Enable C++/CLI (Common Language Runtime)
set_target_properties(ScriptAPI PROPERTIES
    COMMON_LANGUAGE_RUNTIME ""
    VS_GLOBAL_CLRSupport "true"
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    VS_DOTNET_REFERENCES "System;System.Core"
)

# Include directories
target_include_directories(ScriptAPI PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/extern/dotnet/include
    ${CMAKE_SOURCE_DIR}/MyEngine  # ← Can access MyEngine headers
    ${CMAKE_SOURCE_DIR}/MyEngine/include
    ${GLEW_SOURCE_DIR}/include
    $<$<BOOL:${FMOD_API_DIR_FOUND}>:${FMOD_API_DIR_FOUND}/api/core/inc>
    $<$<BOOL:${FMOD_API_DIR_FOUND}>:${FMOD_API_DIR_FOUND}/api/studio/inc>
)

# Output to same directory as MyEngine
set_target_properties(ScriptAPI PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/MyEngine/$<CONFIG>"  # for DLL
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/MyEngine/$<CONFIG>"  # for others i think? 
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/MyEngine/$<CONFIG>"  # for .lib
)
    
# Set folder organization for Visual Studio
set_property(TARGET ScriptAPI PROPERTY FOLDER "")


# Enables multi-processor parallel compilation in Visual Studio
if(MSVC)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif()