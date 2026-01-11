# BaseEngine/CMakeLists.txt
cmake_minimum_required(VERSION 3.20)
project(PulseEngine VERSION 0.1 LANGUAGES C CXX)

# -------------------------------
# CMake / VS defaults
# -------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMakePredefinedTargets")

# Keep runtime consistent (C++/CLI friendly)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

# Put FetchContent downloads in build/libraries/
set(FETCHCONTENT_BASE_DIR "${CMAKE_BINARY_DIR}/libraries")
include(FetchContent)

# Let CMake find your cmake/*.cmake helpers
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

# Disable noisy 3rd-party extras BEFORE fetching deps
set(JSON_BuildTests          OFF CACHE BOOL "Disable nlohmann_json tests" FORCE)
set(RAPIDJSON_BUILD_TESTS    OFF CACHE BOOL "Disable RapidJSON tests"      FORCE)
set(RAPIDJSON_BUILD_EXAMPLES OFF CACHE BOOL "Disable RapidJSON examples"   FORCE)
set(RAPIDJSON_BUILD_DOC      OFF CACHE BOOL "Disable RapidJSON docs"       FORCE)

# Suppress warnings from external libraries (MSVC only)
if(MSVC)
  add_compile_options(
    /experimental:external
    /external:W0
    /external:anglebrackets
  )
endif()

# -------------------------------
# 3rd-party deps (ONCE)
# -------------------------------
include("${CMAKE_CURRENT_LIST_DIR}/cmake/ImportDependencies.cmake")
cmake_policy(PUSH)
set(CMAKE_FOLDER "Libraries")
importDependencies()
cmake_policy(POP)

# Keep your real projects OUTSIDE "Libraries"
set(CMAKE_FOLDER "")

# -------------------------------
# ImGui
# -------------------------------
add_library(imgui
  ${CMAKE_CURRENT_LIST_DIR}/imgui/imgui.cpp
  ${CMAKE_CURRENT_LIST_DIR}/imgui/imgui_draw.cpp
  ${CMAKE_CURRENT_LIST_DIR}/imgui/imgui_widgets.cpp
  ${CMAKE_CURRENT_LIST_DIR}/imgui/imgui_tables.cpp
)
target_include_directories(imgui PUBLIC ${CMAKE_CURRENT_LIST_DIR}/imgui)
set_property(TARGET imgui PROPERTY FOLDER "3rdparty/ImGui")

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

# -------------------------------
# Main projects (ONCE each)
# -------------------------------
add_subdirectory(PulseEngine)     # defines PulseEngine_Editor / PulseEngine_Game
add_subdirectory(ScriptAPI)
add_subdirectory(ManagedScripts)
add_subdirectory(PulseEditor)
add_subdirectory(PulseProtocol)

# -------------------------------
# Nice Solution Explorer layout
# -------------------------------
function(pulse_rootify tgt)
  if(TARGET ${tgt})
    set_target_properties(${tgt} PROPERTIES FOLDER "")
  endif()
endfunction()

pulse_rootify(PulseEditor)
pulse_rootify(PulseProtocol)
pulse_rootify(ScriptAPI)
pulse_rootify(ManagedScripts)

# Visible "PulseEngine" node in the solution (meta target)
if(NOT TARGET PulseEngine)
  add_custom_target(PulseEngine)
  if(TARGET PulseEngine_Editor)
    add_dependencies(PulseEngine PulseEngine_Editor)
  endif()
  if(TARGET PulseEngine_Game)
    add_dependencies(PulseEngine PulseEngine_Game)
  endif()
  set_target_properties(PulseEngine PROPERTIES FOLDER "Engine")
endif()

# --- Hide engine variants by grouping into a folder ---
if(TARGET PulseEngine_Editor)
  set_target_properties(PulseEngine_Editor PROPERTIES FOLDER "Engine")
endif()

if(TARGET PulseEngine_Game)
  set_target_properties(PulseEngine_Game PROPERTIES FOLDER "Engine")
endif()

if(TARGET PulseEngine)
  set_target_properties(PulseEngine PROPERTIES FOLDER "Engine")
endif()

# Default startup project in VS
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT  PulseEditor)
