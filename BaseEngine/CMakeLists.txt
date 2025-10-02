cmake_minimum_required(VERSION 3.20)
project(MyEngine VERSION 0.1 LANGUAGES C CXX)

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

if (COMMAND importDependencies)
  # Everything created inside this block goes to the "Libraries" solution folder
  block(SCOPE_FOR VARIABLES)
    set(CMAKE_FOLDER "Libraries")
    importDependencies()
  endblock()
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
# (safe now because glfw is already imported)
add_library(imgui_backend
  ${CMAKE_CURRENT_LIST_DIR}/imgui/backends/imgui_impl_glfw.cpp
  ${CMAKE_CURRENT_LIST_DIR}/imgui/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui_backend PUBLIC
  ${CMAKE_CURRENT_LIST_DIR}/imgui
  ${CMAKE_CURRENT_LIST_DIR}/imgui/backends
)
target_compile_definitions(imgui_backend PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLEW)
target_link_libraries(imgui_backend PUBLIC imgui glfw)
set_property(TARGET imgui_backend PROPERTY FOLDER "3rdparty/ImGui")

# --- Projects ---
add_subdirectory(MyEngine)

option(ENABLE_EDITOR "Build Editor and compile it into the Engine" ON)
message(STATUS "ENABLE_EDITOR = ${ENABLE_EDITOR}")
if (ENABLE_EDITOR)
  message(STATUS "==> Adding MyEditor subdirectory")
  add_subdirectory(MyEditor)
  target_link_libraries(MyEngine PRIVATE MyEditor)
  target_compile_definitions(MyEngine PRIVATE ENABLE_EDITOR=1)
endif()

# Keep app targets OUTSIDE any folder (root). Do this after they exist.
if(TARGET MyEngine)
  set_property(TARGET MyEngine PROPERTY FOLDER "")
endif()
if(TARGET MyEditor)
  set_property(TARGET MyEditor PROPERTY FOLDER "")
endif()
# (Alternatively, group them under "Apps" instead of root)
# set_property(TARGET MyEngine MyEditor PROPERTY FOLDER "Apps")

# Visual Studio startup project
set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT MyEngine)
