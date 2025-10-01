cmake_minimum_required(VERSION 3.20)
project(MyEngine VERSION 0.1 LANGUAGES C CXX)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
target_link_libraries(imgui_backend PUBLIC imgui glfw) # <-- gives backend access to <GLFW/glfw3.h>
set_property(TARGET imgui_backend PROPERTY FOLDER "3rdparty/ImGui")

# --- Import all 3rd-party deps (GLFW, GLEW, GLM, JSON, FreeType, spdlog) ---
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
include(ImportDependencies OPTIONAL)
include(Utilities OPTIONAL)
if (COMMAND importDependencies)
  importDependencies()
endif()

# --- Projects ---
add_subdirectory(MyEngine)

option(ENABLE_EDITOR "Build Editor and compile it into the Engine" ON)
message(STATUS "ENABLE_EDITOR = ${ENABLE_EDITOR}")
if (ENABLE_EDITOR)
  message(STATUS "==> Adding MyEditor subdirectory")
  add_subdirectory(MyEditor)
  # Link the editor static lib *into* the engine exe
  target_link_libraries(MyEngine PRIVATE MyEditor)
  target_compile_definitions(MyEngine PRIVATE ENABLE_EDITOR=1)
endif()

# Visual Studio startup project
set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT MyEngine)
