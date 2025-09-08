include(FetchContent)

# Macro to import GLFW
macro(import_glfw)
 if(NOT TARGET glfw)
        FetchContent_Declare(
            glfw
            GIT_REPOSITORY https://github.com/glfw/glfw.git
            GIT_TAG 3.3.8
        )
        # Trim extras
        set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
        set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
        set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

        # Works on CMake >=3.14, including 3.29.2 and all 4.x
        FetchContent_MakeAvailable(glfw)
        # no add_subdirectory(), no include_directories()
    endif()
endmacro()

# --- GLEW ---
macro(import_glew)
    if(NOT TARGET GLEW::glew AND NOT TARGET glew)
        FetchContent_Declare(
            glew
            GIT_REPOSITORY https://github.com/Perlmint/glew-cmake
            GIT_TAG master
        )
        FetchContent_MakeAvailable(glew)
        # no include_directories()
        # Link with: target_link_libraries(<tgt> PRIVATE GLEW::glew)  (or 'glew'/'glew_s' if that's what the repo defines)
    endif()
endmacro()

# --- GLM ---
macro(import_glm)
    if(NOT TARGET glm AND NOT TARGET glm::glm)
        FetchContent_Declare(
            glm
            GIT_REPOSITORY https://github.com/g-truc/glm.git
            GIT_TAG 0.9.9.8
        )
        FetchContent_MakeAvailable(glm)
        # Link with: target_link_libraries(<tgt> PRIVATE glm)  # or glm::glm if available
    endif()
endmacro()

# Macro to import all dependencies
macro(importDependencies)
    import_glfw()
    import_glew()
    import_glm()
endmacro()