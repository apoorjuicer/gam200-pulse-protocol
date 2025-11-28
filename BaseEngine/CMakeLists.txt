cmake_minimum_required(VERSION 3.20)
project(MyEngine VERSION 0.1 LANGUAGES C CXX)
set(JSON_BuildTests OFF CACHE BOOL "Disable nlohmann_json tests" FORCE)
set(RAPIDJSON_BUILD_TESTS    OFF CACHE BOOL "Disable RapidJSON tests"    FORCE)
set(RAPIDJSON_BUILD_EXAMPLES OFF CACHE BOOL "Disable RapidJSON examples" FORCE)
set(RAPIDJSON_BUILD_DOC      OFF CACHE BOOL "Disable RapidJSON docs"     FORCE)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

# Suppress warnings from external libraries (MSVC only)
if(MSVC)
    add_compile_options(
        /experimental:external
        /external:W0
        /external:anglebrackets
    )
endif()

# Put FetchContent downloads in build/libraries/
set(FETCHCONTENT_BASE_DIR "${CMAKE_BINARY_DIR}/libraries")
include(FetchContent)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Import all 3rd-party deps (GLFW, GLEW, GLM, JSON, FreeType, spdlog) ---
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

include(ImportDependencies OPTIONAL)
include(Utilities OPTIONAL)

# CMake 3.20-compatible replacement for block()/endblock()
if (COMMAND importDependencies)
    cmake_policy(PUSH)

    # Everything created here goes into the "Libraries" folder in Visual Studio
    set(CMAKE_FOLDER "Libraries")

    importDependencies()

    cmake_policy(POP)
endif()

# --- ImGui core ---
add_library(imgui
    ${CMAKE_CURRENT_LIST_DIR}/imgui/imgui.cpp
    ${CMAKE_CURRENT_LIST_DIR}/imgui/imgui_draw.cpp
    ${CMAKE_CURRENT_LIST_DIR}/imgui/imgui_widgets.cpp
    ${CMAKE_CURRENT_LIST_DIR}/imgui/imgui_tables.cpp
)
target_include_directories(imgui PUBLIC ${CMAKE_CURRENT_LIST_DIR}/imgui)
set_property(TARGET imgui PROPERTY FOLDER "3rdparty/ImGui")

# --- ImGui backends (GLFW + OpenGL3) ---
add_library(imgui_backend
    ${CMAKE_CURRENT_LIST_DIR}/imgui/backends/imgui_impl_glfw.cpp
    ${CMAKE_CURRENT_LIST_DIR}/imgui/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui_backend PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/imgui
    ${CMAKE_CURRENT_LIST_DIR}/imgui/backends
)
target_compile_definitions(imgui_backend PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLEW)
target_link_libraries(imgui_backend PRIVATE imgui glfw)
set_property(TARGET imgui_backend PROPERTY FOLDER "3rdparty/ImGui")

# --- Projects ---
add_subdirectory(MyEngine)

option(ENABLE_EDITOR "Build the Editor executable" ON)
message(STATUS "ENABLE_EDITOR = ${ENABLE_EDITOR}")

if (ENABLE_EDITOR)
    add_subdirectory(MyEditor)
    target_link_libraries(MyEditor PRIVATE MyEngine)
    target_compile_definitions(MyEngine PUBLIC ENABLE_EDITOR)
    target_compile_definitions(MyEditor PUBLIC ENABLE_EDITOR)
else()
    # Build game only
    add_subdirectory(MyGame)
endif()
# Keep app targets OUTSIDE any folder
if(TARGET MyEngine)
    set_property(TARGET MyEngine PROPERTY FOLDER "")
endif()
if(TARGET MyEditor)
    set_property(TARGET MyEditor PROPERTY FOLDER "")
endif()
if(TARGET MyGame)
    set_property(TARGET MyGame PROPERTY FOLDER "")
endif()

# -------------------------------
# Visual Studio startup project
# -------------------------------
if (ENABLE_EDITOR)
    set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT MyEditor)
else()
    set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT MyGame)
endif()

# -------------------------------
# Hide console window in Release for MyGame
# Show console in Debug
# -------------------------------
if (WIN32)

    if (TARGET MyGame)
        target_link_options(MyGame PRIVATE 
            "$<$<CONFIG:Debug>:/SUBSYSTEM:CONSOLE>"
            "$<$<CONFIG:Release>:/SUBSYSTEM:WINDOWS>"
        )
    endif()

endif()

