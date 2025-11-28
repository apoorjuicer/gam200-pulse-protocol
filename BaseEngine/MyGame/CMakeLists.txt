cmake_minimum_required(VERSION 3.20)

set(GAME_NAME MyGame)
set(GAME_ROOT ${CMAKE_CURRENT_LIST_DIR})

# Grab all cpp/h in this folder
file(GLOB_RECURSE GAME_SRC CONFIGURE_DEPENDS
    "${GAME_ROOT}/*.cpp"
    "${GAME_ROOT}/*.h"
) 

add_executable(${GAME_NAME} ${GAME_SRC})

# Ensures POST_BUILD copy commands run
set_target_properties(${GAME_NAME} PROPERTIES 
    VS_GLOBAL_DisableFastUpToDateCheck true
)

# Use the engine library
target_link_libraries(${GAME_NAME} PRIVATE 
    MyEngine 
)

# So we can do #include "pch/pch.h" and other engine headers
target_include_directories(${GAME_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/MyEngine
)

add_dependencies(MyGame ScriptAPI ManagedScripts)

# Copy MyEngine.dll to MyGame output
add_custom_command(TARGET ${GAME_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:MyEngine>"
        "$<TARGET_FILE_DIR:${GAME_NAME}>"
    COMMENT "Copying MyEngine.dll to MyGame output directory"
)

# Copy ScriptAPI.dll to MyGame output
add_custom_command(TARGET ${GAME_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_BINARY_DIR}/MyEngine/$<CONFIG>/ScriptAPI.dll"
        "$<TARGET_FILE_DIR:${GAME_NAME}>/ScriptAPI.dll"
    COMMENT "Copying ScriptAPI.dll to MyGame"
)

# Copy ManagedScripts.dll to MyGame output
add_custom_command(TARGET ${GAME_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_BINARY_DIR}/MyEngine/$<CONFIG>/ManagedScripts.dll"
        "$<TARGET_FILE_DIR:${GAME_NAME}>/ManagedScripts.dll"
    COMMENT "Copying ManagedScripts.dll to MyGame"
)

# Copy .NET Runtime DLLs for C# scripting
add_custom_command(TARGET ${GAME_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/extern/dotnet/bin"
        "$<TARGET_FILE_DIR:${GAME_NAME}>"
    COMMENT "Copying .NET Runtime DLLs to MyGame output directory"
)

# Copy JSON
add_custom_command(TARGET ${GAME_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/MyEngine/JSON
        $<TARGET_FILE_DIR:${GAME_NAME}>/JSON
    COMMENT "Copying JSON files to MyGame"
)

# Copy Assets
add_custom_command(TARGET ${GAME_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/MyEngine/Assets
        $<TARGET_FILE_DIR:${GAME_NAME}>/Assets
    COMMENT "Copying Assets to MyGame"
)

# Automatically copies all third-party runtime DLLs(GLFW) that MyGame depends on to the executable directory
add_custom_command(TARGET ${GAME_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_RUNTIME_DLLS:${GAME_NAME}>"
        "$<TARGET_FILE_DIR:${GAME_NAME}>"
    COMMAND_EXPAND_LISTS
    COMMENT "Copying DLL Dependencies like glfw3.dll to editor executable directory."
    VERBATIM
)

#Enables multi-processor parallel compilation in Visual Studio
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
    
    # --- FMOD DLL copy for MyGame (Debug/Release aware) ---
    if (FMOD_API_DIR_FOUND)

    set(FMOD_CORE_DEBUG     "${FMOD_API_DIR_FOUND}/api/core/lib/x64/fmodL.dll")
    set(FMOD_CORE_RELEASE   "${FMOD_API_DIR_FOUND}/api/core/lib/x64/fmod.dll")

    set(FMOD_STUDIO_DEBUG   "${FMOD_API_DIR_FOUND}/api/studio/lib/x64/fmodstudioL.dll")
    set(FMOD_STUDIO_RELEASE "${FMOD_API_DIR_FOUND}/api/studio/lib/x64/fmodstudio.dll")

    add_custom_command(TARGET ${GAME_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<IF:$<CONFIG:Debug>,${FMOD_CORE_DEBUG},${FMOD_CORE_RELEASE}>"
            $<TARGET_FILE_DIR:${GAME_NAME}>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<IF:$<CONFIG:Debug>,${FMOD_STUDIO_DEBUG},${FMOD_STUDIO_RELEASE}>"
            $<TARGET_FILE_DIR:${GAME_NAME}>
    )
    endif()
endif()