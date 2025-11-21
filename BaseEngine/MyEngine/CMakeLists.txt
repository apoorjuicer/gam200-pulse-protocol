set(ENGINE_NAME MyEngine)

# Auto-collect sources (keeps Solution Explorer in sync)
file(GLOB_RECURSE ENGINE_SRC CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_LIST_DIR}/**/*.c"
    "${CMAKE_CURRENT_LIST_DIR}/**/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/**/*.h"
    "${CMAKE_CURRENT_LIST_DIR}/**/*.hpp"
)

# Make SHARED to be DLL instead of lib
add_library(${ENGINE_NAME} SHARED ${ENGINE_SRC})

# CRITICAL: Export all symbols automatically
#set_target_properties(${ENGINE_NAME} PROPERTIES
#    WINDOWS_EXPORT_ALL_SYMBOLS ON
#)

# Define DLL export macro
target_compile_definitions(${ENGINE_NAME} PRIVATE DLL_API_EXPORT)

if (ENABLE_EDITOR)
    target_compile_definitions(MyEngine PUBLIC ENABLE_EDITOR)
endif()

source_group(TREE "${CMAKE_CURRENT_LIST_DIR}" FILES ${ENGINE_SRC})

# C++ standard / warnings
set_property(TARGET ${ENGINE_NAME} PROPERTY CXX_STANDARD 20)
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${ENGINE_NAME} PRIVATE -Wall)
elseif (MSVC)
    target_compile_options(${ENGINE_NAME} PRIVATE /W4 /WX-)
endif()

# Define for static GLEW usage
target_compile_definitions(${ENGINE_NAME} PUBLIC GLEW_STATIC)

# Includes owned by this target (3rd-party includes are carried by their targets)
target_include_directories(${ENGINE_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}   # Engine headers
    "${CMAKE_SOURCE_DIR}/extern/dotnet/include"  # .NET Core hosting headers
)

# ---- Robust FMOD discovery (once) ----
if (NOT DEFINED FMOD_API_DIR_FOUND)
  set(_FMOD_CANDIDATES
      "${FMOD_API_DIR}"                                   # user-provided -DFMOD_API_DIR=...
      #"${CMAKE_CURRENT_LIST_DIR}/../FMOD_API"             # project root / FMOD_API
      "${CMAKE_CURRENT_LIST_DIR}/FMOD_API"             # project root / FMOD_API
      "${CMAKE_CURRENT_LIST_DIR}/FMOD_API"                # MyEngine/FMOD_API
      "${CMAKE_SOURCE_DIR}/FMOD_API"                      # top-level / FMOD_API
      "${CMAKE_SOURCE_DIR}/ThirdParty/FMOD_API"           # typical thirdparty layout
  )
  foreach(_cand IN LISTS _FMOD_CANDIDATES)
    if(_cand AND EXISTS "${_cand}/api/core/lib/x64")
      set(FMOD_API_DIR_FOUND "${_cand}" CACHE PATH "FMOD API root" FORCE)
      break()
    endIF()
  endforeach()
  if(NOT FMOD_API_DIR_FOUND)
    message(FATAL_ERROR "FMOD_API folder not found. Set -DFMOD_API_DIR=PATH (must contain api/core/lib/x64).")
  endif()
endif()
message(STATUS "Using FMOD at: ${FMOD_API_DIR_FOUND}")

# FMOD includes
target_include_directories(${ENGINE_NAME} PUBLIC
    "${FMOD_API_DIR_FOUND}/api/core/inc"
    "${FMOD_API_DIR_FOUND}/api/studio/inc"
)

# Core deps (PUBLIC so MyEditor inherits them via MyEngine link)
target_link_libraries(${ENGINE_NAME} PUBLIC
    glfw
    libglew_static
    imgui
    imgui_backend
    Freetype::Freetype
)

# FMOD libraries (multi-config safe for MSVC)
target_link_libraries(${ENGINE_NAME} PUBLIC
    debug     "${FMOD_API_DIR_FOUND}/api/core/lib/x64/fmodL_vc.lib"
    optimized "${FMOD_API_DIR_FOUND}/api/core/lib/x64/fmod_vc.lib"
    debug     "${FMOD_API_DIR_FOUND}/api/studio/lib/x64/fmodstudioL_vc.lib"
    optimized "${FMOD_API_DIR_FOUND}/api/studio/lib/x64/fmodstudio_vc.lib"
)

# Copy the correct FMOD DLLs next to the EXE
add_custom_command(TARGET ${ENGINE_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${FMOD_API_DIR_FOUND}/api/core/lib/x64/$<IF:$<CONFIG:Debug>,fmodL.dll,fmod.dll>"
    $<TARGET_FILE_DIR:${ENGINE_NAME}>
  )
add_custom_command(TARGET ${ENGINE_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${FMOD_API_DIR_FOUND}/api/studio/lib/x64/$<IF:$<CONFIG:Debug>,fmodstudioL.dll,fmodstudio.dll>"
    $<TARGET_FILE_DIR:${ENGINE_NAME}>
  )

  # Copy JSON config files to build directory
add_custom_command(TARGET ${ENGINE_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/JSON
    ${CMAKE_BINARY_DIR}/MyEngine/JSON
    COMMENT "Copying JSON config files to build directory"
)

# Copy Assets to build directory
add_custom_command(TARGET ${ENGINE_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/Assets
    ${CMAKE_BINARY_DIR}/MyEngine/Assets
    COMMENT "Copying Assets to build directory"
)

# Copy .NET Runtime DLLs (post-build event that copies these DLLs into the output directory.)
add_custom_command(TARGET MyEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/extern/dotnet/bin"
        "$<TARGET_FILE_DIR:MyEngine>"
    COMMENT "Copying .NET Runtime DLLs to MyEngine output directory"
)

# MyEngine wait for ManagedScripts to build first and include it in the build dependencies
#if(TARGET ManagedScripts)
# add_dependencies(MyEngine ManagedScripts)
#endif()

#if(TARGET ScriptAPI)
#    add_dependencies(MyEngine ScriptAPI)
#endif()


# (THESE ARE ONLY FOR EXE)
# Automatically copies all third-party runtime DLLs(GLFW) that MyEditor depends on to the executable directory 
# add_custom_command(TARGET MyEngine POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E copy_if_different
#         "$<TARGET_RUNTIME_DLLS:MyEngine>"
#         "$<TARGET_FILE_DIR:MyEngine>"
#     COMMAND_EXPAND_LISTS
#     COMMENT "Copying DLL Dependencies like ManagedScript.dll to editor executable directory."
#     VERBATIM
# )

# # Copy ScriptAPI.dll to MyEngine output
# add_custom_command(TARGET MyEngine POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E copy_if_different
#         "$<TARGET_FILE:ScriptAPI>"
#         "$<TARGET_FILE_DIR:MyEngine>"
#     COMMENT "Copying ScriptAPI.dll to MyEngine output directory"
# )

# Enables multi-processor parallel compilation in Visual Studio
if(MSVC)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif()