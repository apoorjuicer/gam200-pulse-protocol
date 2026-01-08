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
#add_dependencies(ScriptAPI PulseEngine)

target_link_libraries(ScriptAPI
    PRIVATE
        PulseEngine
)

# Enable C++/CLI (Common Language Runtime)
set_target_properties(ScriptAPI PROPERTIES
    COMMON_LANGUAGE_RUNTIME ""
    #VS_GLOBAL_CLRSupport "true"
    VS_GLOBAL_CLRSupport "NetCore"
    VS_GLOBAL_TargetFramework "net8.0"
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    #VS_DOTNET_REFERENCES "System;System.Core"
)

# Force /clr:netcore compiler flag
if(MSVC)
    target_compile_options(ScriptAPI PRIVATE "/clr:netcore")
endif()

# Include directories
target_include_directories(ScriptAPI PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/extern/dotnet/include
    ${CMAKE_SOURCE_DIR}/PulseEngine  
    ${CMAKE_SOURCE_DIR}/PulseEngine/include
    ${GLEW_SOURCE_DIR}/include
    $<$<BOOL:${FMOD_API_DIR_FOUND}>:${FMOD_API_DIR_FOUND}/api/core/inc>
    $<$<BOOL:${FMOD_API_DIR_FOUND}>:${FMOD_API_DIR_FOUND}/api/studio/inc>
)

# Output to same directory as PulseEngine
#set_target_properties(ScriptAPI PROPERTIES
#    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/PulseEngine/$<CONFIG>"  # for DLL
#    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/PulseEngine/$<CONFIG>"  # for others i think? 
#    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/PulseEngine/$<CONFIG>"  # for .lib
#)

# Output to .bin directory to match guide
set_target_properties(ScriptAPI PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/.bin/$<CONFIG>-x64"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/.bin/$<CONFIG>-x64"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/.bin/$<CONFIG>-x64"
)
    
# Set folder organization for Visual Studio
set_property(TARGET ScriptAPI PROPERTY FOLDER "")


# Enables multi-processor parallel compilation in Visual Studio
if(MSVC)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif()