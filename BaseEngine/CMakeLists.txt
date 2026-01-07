cmake_minimum_required(VERSION 3.20)
project(PulseEngine VERSION 0.1 LANGUAGES C CXX)
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

# --- Import all 3rd-party deps (GLFW, GLEW, GLM, JSON, FreeType) ---
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



# --------------- C# Scripting Support ---------------
add_subdirectory(ScriptAPI)
#add_subdirectory(ManagedScripts)


add_subdirectory(PulseEngine)
# if(TARGET ScriptAPI)
#     add_dependencies(PulseEngine ScriptAPI)
# endif()
# if(TARGET ManagedScripts)
#     add_dependencies(PulseEngine ManagedScripts)
# endif()

option(ENABLE_EDITOR "Build the Editor executable" ON)
message(STATUS "ENABLE_EDITOR = ${ENABLE_EDITOR}")



if (ENABLE_EDITOR)
    add_subdirectory(PulseEditor)
    target_link_libraries(PulseEditor PRIVATE PulseEngine )
    #target_link_libraries(PulseEditor PRIVATE PulseEngine ScriptAPI)
    
    target_compile_definitions(PulseEngine PUBLIC ENABLE_EDITOR)
    target_compile_definitions(PulseEditor PUBLIC ENABLE_EDITOR)

    add_dependencies(PulseEditor ScriptAPI)

#endif()
# Add the game executable (no editor)
#add_subdirectory(PulseProtocol)

# this is test when merging
else()
    # Build game only
    add_subdirectory(PulseProtocol)
endif()




# Keep app targets OUTSIDE any folder
if(TARGET PulseEngine)
    set_property(TARGET PulseEngine PROPERTY FOLDER "")
endif()
if(TARGET PulseEditor)
    set_property(TARGET PulseEditor PROPERTY FOLDER "")
endif()
if(TARGET PulseProtocol)
    set_property(TARGET PulseProtocol PROPERTY FOLDER "")
endif()

# -------------------------------
# Visual Studio startup project
# -------------------------------
if (ENABLE_EDITOR)
    set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT PulseEditor)
else()
    set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT PulseProtocol)
endif()

# -------------------------------
# Hide console window in Release for PulseProtocol
# Show console in Debug
# -------------------------------
if (WIN32)

    if (TARGET PulseProtocol)
        target_link_options(PulseProtocol PRIVATE 
            "$<$<CONFIG:Debug>:/SUBSYSTEM:CONSOLE>"
            "$<$<CONFIG:Release>:/SUBSYSTEM:WINDOWS>"
        )
    endif()

endif()

