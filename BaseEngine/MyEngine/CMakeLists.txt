set(ENGINE_NAME MyEngine)
set(EXPORT_INSTALLER_NAME "Find${ENGINE_NAME}")

file(GLOB_RECURSE GRAPHICS_ABSTRACTION_LAYER_SRC_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} 
    ${CMAKE_CURRENT_LIST_DIR}/*.[ch]pp
    ${CMAKE_CURRENT_LIST_DIR}/*.h
)

set(ALL_FILES
    ${GRAPHICS_ABSTRACTION_LAYER_SRC_FILES} 
)

# Call the macro to group files by folder
GROUP_FILES_BY_FOLDER(${ALL_FILES})

# Create the executable (or library)
add_executable(${ENGINE_NAME} ${ALL_FILES})

set_property(TARGET ${ENGINE_NAME}  PROPERTY CXX_STANDARD 20) 
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${ENGINE_NAME} PRIVATE -Wall ${DisableWarnings})
elseif (MSVC)
    target_compile_options(${ENGINE_NAME} PRIVATE /W4 /WX-)
endif()

target_include_directories(${ENGINE_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(${ENGINE_NAME} PUBLIC ${GLEW_SOURCE_DIR}/include)

# Link GLFW and GLEW to the engine
target_link_libraries(${ENGINE_NAME} PUBLIC glfw libglew_static)

# Link Freetype to the engine
target_link_libraries(MyEngine PRIVATE Freetype::Freetype)

# Include directories for FMOD
target_include_directories(${ENGINE_NAME} PUBLIC 
    ${CMAKE_CURRENT_LIST_DIR}/FMOD_API/api/core/inc
    ${CMAKE_CURRENT_LIST_DIR}/FMOD_API/api/studio/inc
)

target_compile_definitions(MyEngine PRIVATE
  ASSETS_DIR="${CMAKE_SOURCE_DIR}/MyEngine/Assets"
)

# Add FMOD libraries based on build type
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_link_libraries(${ENGINE_NAME} PUBLIC 
        ${CMAKE_CURRENT_LIST_DIR}/FMOD_API/api/core/lib/x64/fmodL_vc.lib
        ${CMAKE_CURRENT_LIST_DIR}/FMOD_API/api/studio/lib/x64/fmodstudioL_vc.lib
    )
else() # Release or other build types
    target_link_libraries(${ENGINE_NAME} PUBLIC 
        ${CMAKE_CURRENT_LIST_DIR}/FMOD_API/api/core/lib/x64/fmod_vc.lib
        ${CMAKE_CURRENT_LIST_DIR}/FMOD_API/api/studio/lib/x64/fmodstudio_vc.lib
    )
endif()


# Copy FMOD DLLs after build (Windows only)
if (WIN32)
    add_custom_command(TARGET ${ENGINE_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
	"${CMAKE_CURRENT_LIST_DIR}/FMOD_API/api/core/lib/x64/fmod.dll"
	"${CMAKE_CURRENT_LIST_DIR}/FMOD_API/api/studio/lib/x64/fmodstudio.dll"
        $<TARGET_FILE_DIR:${ENGINE_NAME}>
    )
endif()
