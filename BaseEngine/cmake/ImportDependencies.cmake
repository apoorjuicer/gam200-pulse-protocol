include(FetchContent)

# Macro to import GLFW
macro(import_glfw)
    if(NOT TARGET glfw)  # Guard to prevent multiple inclusion
        FetchContent_Declare(
            glfw
            GIT_REPOSITORY https://github.com/glfw/glfw.git
            GIT_TAG 3.3.8
        )
        if(NOT glfw_POPULATED)
            set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
            set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
            set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
            FetchContent_Populate(glfw)
        endif()

        add_subdirectory(${glfw_SOURCE_DIR})
        include_directories(${GLFW_SOURCE_DIR}/include)
    endif()
endmacro()

macro(import_glew)
    FetchContent_Declare(
      glew
      GIT_REPOSITORY https://github.com/Perlmint/glew-cmake
      GIT_TAG master
    )

    FetchContent_MakeAvailable(glew)
    include_directories(${GLEW_SOURCE_DIR}/include)
endmacro()


# Macro to import all dependencies
macro(importDependencies)
    import_glfw()
    import_glew()
endmacro()