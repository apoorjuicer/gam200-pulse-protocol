set(EDITOR_NAME MyEditor)
set(EDITOR_ROOT ${CMAKE_CURRENT_LIST_DIR})

message(STATUS "Configuring ${EDITOR_NAME} from: ${EDITOR_ROOT}")

# Pick up editor sources (and headers for VS filters)
file(GLOB_RECURSE EDITOR_SRC CONFIGURE_DEPENDS
    "${EDITOR_ROOT}/*.c"
    "${EDITOR_ROOT}/*.cc"
    "${EDITOR_ROOT}/*.cxx"
    "${EDITOR_ROOT}/*.cpp"
    "${EDITOR_ROOT}/*.mm"
    "${EDITOR_ROOT}/*.m"
    "${EDITOR_ROOT}/*.h"
    "${EDITOR_ROOT}/*.hpp"
)

# Always create the target (even if empty) so it shows in Solution Explorer
if (NOT EDITOR_SRC)
    message(WARNING "[${EDITOR_NAME}] No source files found. Creating __dummy_editor.cpp so the target shows up.")
    file(WRITE "${EDITOR_ROOT}/__dummy_editor.cpp" "// generated dummy file to satisfy CMake\nint __myeditor_dummy__ = 0;")
    list(APPEND EDITOR_SRC "${EDITOR_ROOT}/__dummy_editor.cpp")
endif()

add_executable(${EDITOR_NAME} ${EDITOR_SRC})

target_compile_definitions(MyEditor PUBLIC ENABLE_EDITOR)

target_link_libraries(${EDITOR_NAME}
    PRIVATE
        MyEngine          # depends on the engine
    PUBLIC
        imgui
        imgui_backend
        glfw
        libglew_static
        Freetype::Freetype
)

# Nice Solution Explorer layout
source_group(TREE "${EDITOR_ROOT}" FILES ${EDITOR_SRC})

# Editor needs to include engine headers, and often uses GLEW/FMOD types via those headers
# (FMOD path: reuse the same detection if not already set)
if (NOT DEFINED FMOD_API_DIR_FOUND)
  set(_FMOD_CANDIDATES
      "${FMOD_API_DIR}"
      "${CMAKE_SOURCE_DIR}/MyEngine/../FMOD_API"
      "${CMAKE_SOURCE_DIR}/FMOD_API"
      "${CMAKE_SOURCE_DIR}/ThirdParty/FMOD_API"
  )
  foreach(_cand IN LISTS _FMOD_CANDIDATES)
    if(_cand AND EXISTS "${_cand}/api/core/lib/x64")
      set(FMOD_API_DIR_FOUND "${_cand}" CACHE PATH "FMOD API root" FORCE)
      break()
    endif()
  endforeach()
  if(NOT FMOD_API_DIR_FOUND)
    message(WARNING "[${EDITOR_NAME}] FMOD_API not found here; assuming Engine target carries it. If you see FMOD compile errors, set -DFMOD_API_DIR=...")
  endif()
endif()

target_include_directories(${EDITOR_NAME} PUBLIC
    ${CMAKE_SOURCE_DIR}/MyEngine
    ${CMAKE_SOURCE_DIR}/MyEngine/include
    ${GLEW_SOURCE_DIR}/include
    $<$<BOOL:${FMOD_API_DIR_FOUND}>:${FMOD_API_DIR_FOUND}/api/core/inc>
    $<$<BOOL:${FMOD_API_DIR_FOUND}>:${FMOD_API_DIR_FOUND}/api/studio/inc>
)

# Link what the editor needs (inherits more via the Engine when linked into it)
target_link_libraries(${EDITOR_NAME} PUBLIC
    imgui
    imgui_backend
    glfw
    libglew_static
    Freetype::Freetype
)

# Keep GLEW usage consistent
target_compile_definitions(${EDITOR_NAME} PUBLIC GLEW_STATIC)

# Warnings
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${EDITOR_NAME} PRIVATE -Wall)
elseif (MSVC)
    target_compile_options(${EDITOR_NAME} PRIVATE /W4 /WX-)
endif()

message(STATUS "[${EDITOR_NAME}] sources: ${EDITOR_SRC}")
# Debug DLL
add_custom_command(TARGET MyEditor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${FMOD_API_DIR_FOUND}/api/core/lib/x64/fmodL.dll"
    $<TARGET_FILE_DIR:MyEditor>
)

# Release DLL
add_custom_command(TARGET MyEditor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${FMOD_API_DIR_FOUND}/api/core/lib/x64/fmod.dll"
    $<TARGET_FILE_DIR:MyEditor>
)
