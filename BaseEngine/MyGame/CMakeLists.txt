cmake_minimum_required(VERSION 3.20)

set(GAME_NAME MyGame)
set(GAME_ROOT ${CMAKE_CURRENT_LIST_DIR})

# Grab all cpp/h in this folder
file(GLOB_RECURSE GAME_SRC CONFIGURE_DEPENDS
    "${GAME_ROOT}/*.cpp"
    "${GAME_ROOT}/*.h"
)

add_executable(${GAME_NAME} ${GAME_SRC})

# ===== Rename Release executable to PulseProtocol.exe =====
set_target_properties(${GAME_NAME} PROPERTIES
    OUTPUT_NAME_RELEASE "PulseProtocol"
)

set_target_properties(${GAME_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/MyGame/Release/PulseProtocol/Game"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_BINARY_DIR}/MyGame/Debug/PulseProtocol/Game"
)

# Use the engine library
target_link_libraries(${GAME_NAME} PRIVATE MyEngine)

# So we can do #include "pch/pch.h" and other engine headers
target_include_directories(${GAME_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/MyEngine
)

# --- FMOD DLL copy for MyGame (Debug/Release aware) ---
if (MSVC AND FMOD_API_DIR_FOUND)

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

# ---- COPY JSON FOLDER FOR RUNTIME (installer-friendly) ----
add_custom_command(TARGET ${GAME_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/MyEngine/JSON"
        "$<TARGET_FILE_DIR:${GAME_NAME}>/../../MyEngine/JSON"
)

# ---- COPY ANY OTHER ASSETS FOLDER (optional, recommended) ----
add_custom_command(TARGET ${GAME_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/MyEngine/Assets"
        "$<TARGET_FILE_DIR:${GAME_NAME}>/../../MyEngine/Assets"
)

add_custom_command(TARGET ${GAME_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/MyEngine/Audio"
        "$<TARGET_FILE_DIR:${GAME_NAME}>/../../MyEngine/Audio"
)

# ---- COPY SHADER FOLDER ----
add_custom_command(TARGET ${GAME_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/MyEngine/Graphics/shader"
        "$<TARGET_FILE_DIR:${GAME_NAME}>/../../MyEngine/Graphics/shader"
)
