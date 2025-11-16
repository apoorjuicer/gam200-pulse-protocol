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
target_link_libraries(${GAME_NAME} PRIVATE MyEngine)

# So we can do #include "pch/pch.h" and other engine headers
target_include_directories(${GAME_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/MyEngine
)

# --- FMOD DLL copy for MyGame (must be in this file) ---
if (MSVC AND FMOD_API_DIR_FOUND)
    add_custom_command(TARGET ${GAME_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${FMOD_API_DIR_FOUND}/api/core/lib/x64/fmodL.dll"
            "${FMOD_API_DIR_FOUND}/api/studio/lib/x64/fmodstudioL.dll"
            $<TARGET_FILE_DIR:${GAME_NAME}>
    )
endif()