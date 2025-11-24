cmake_minimum_required(VERSION 3.20)

set(GAME_NAME MyGame)
set(GAME_ROOT ${CMAKE_CURRENT_LIST_DIR})

# Grab all cpp/h in this folder
file(GLOB_RECURSE GAME_SRC CONFIGURE_DEPENDS
    "${GAME_ROOT}/*.cpp"
    "${GAME_ROOT}/*.h"
) 

add_executable(${GAME_NAME} ${GAME_SRC})

# Use the engine library
target_link_libraries(${GAME_NAME} PRIVATE 
    MyEngine 
    ScriptAPI
)

# So we can do #include "pch/pch.h" and other engine headers
target_include_directories(${GAME_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/MyEngine
)

add_dependencies(MyGame ScriptAPI)

# Copy MyEngine.dll to MyGame output
add_custom_command(TARGET ${GAME_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:MyEngine>"
        "$<TARGET_FILE_DIR:${GAME_NAME}>"
    COMMENT "Copying MyEngine.dll to MyGame output directory"
)

# Copy .NET Runtime DLLs for C# scripting
add_custom_command(TARGET ${GAME_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/extern/dotnet/bin"
        "$<TARGET_FILE_DIR:${GAME_NAME}>"
    COMMENT "Copying .NET Runtime DLLs to MyGame output directory"
)

# Copy Assets and JSON
add_custom_command(TARGET ${GAME_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/MyEngine/JSON
        $<TARGET_FILE_DIR:${GAME_NAME}>/JSON
    COMMENT "Copying JSON files to MyGame"
)


# Copy MyEngine.dll and all its output to MyGame
# add_custom_command(TARGET MyGame POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E copy_directory
#         "${CMAKE_BINARY_DIR}/MyEngine/$<CONFIG>"
#         "$<TARGET_FILE_DIR:MyGame>"
#     COMMENT "Copying MyEngine.dll, ScriptAPI.dll, ManagedScripts.dll to MyGame"
# )

# Automatically copies all third-party runtime DLLs(GLFW) that MyGame depends on to the executable directory
add_custom_command(TARGET ${GAME_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_RUNTIME_DLLS:${GAME_NAME}>"
        "$<TARGET_FILE_DIR:${GAME_NAME}>"
    COMMAND_EXPAND_LISTS
    COMMENT "Copying DLL Dependencies like glfw3.dll to editor executable directory."
    VERBATIM
)

# build these two first, and include them in the project dependencies
# if(TARGET ScriptAPI)
#     add_dependencies(${GAME_NAME} ScriptAPI)
# endif()
# if(TARGET ManagedScripts)
#     add_dependencies(${GAME_NAME} ManagedScripts)
# endif()

#Enables multi-processor parallel compilation in Visual Studio
if(MSVC)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif()