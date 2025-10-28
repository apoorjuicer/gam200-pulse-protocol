set(CMAKE_FOLDER "Libraries")
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
    #currently not in use due to broken GLEW master branch on github
        #FetchContent_Declare(
        #    glew
        #    GIT_REPOSITORY https://github.com/Perlmint/glew-cmake
        #    GIT_TAG master
        #)

       FetchContent_Declare(
            glew
            GIT_REPOSITORY https://github.com/Perlmint/glew-cmake.git
            GIT_TAG d06782b910213d675925e6e51a69ad0fd1fe1f23
        )
        FetchContent_MakeAvailable(glew)
        # no include_directories()
        # Link with: target_link_libraries(<tgt> PRIVATE GLEW::glew)  (or 'glew'/'glew_s' if that's what the repo defines)
    endif()
endmacro()

# Macro to import glm
macro(import_glm)
    if(NOT TARGET glm)  # Guard to prevent multiple inclusion
        FetchContent_Declare(
            glm
            GIT_REPOSITORY https://github.com/g-truc/glm.git
            GIT_TAG 0.9.9.8
        )
        FetchContent_MakeAvailable(glm)
 
        include_directories(${glm_SOURCE_DIR})
    endif()
endmacro()

macro(import_json)
    if(NOT TARGET json)  # Guard to prevent multiple inclusion
        FetchContent_Declare(
            json
            GIT_REPOSITORY "https://github.com/Tencent/rapidjson.git"
            GIT_TAG v1.1.0
        )
        FetchContent_MakeAvailable(json)
 
        include_directories(${json_SOURCE_DIR}/include)
    endif()
endmacro()

# Import FreeType once and expose target Freetype::Freetype
macro(import_freetype)
  if(NOT TARGET Freetype::Freetype)
    # Prefer a preinstalled package (vcpkg, brew, system)
    find_package(Freetype QUIET CONFIG)
    if(NOT Freetype_FOUND)
      # Some environments only have FindFreetype.cmake
      find_package(Freetype QUIET)
    endif()

    if(NOT Freetype_FOUND AND NOT TARGET Freetype::Freetype)
      include(FetchContent)
      # Fetch FreeType source
      FetchContent_Declare(
        freetype_src
        GIT_REPOSITORY https://github.com/freetype/freetype.git
        GIT_TAG        VER-2-13-3
        GIT_SHALLOW    TRUE
      )

      # Trim features to avoid extra dependencies (zlib/png/brotli/bzip2)
      set(FT_DISABLE_BZIP2     ON  CACHE BOOL "" FORCE)
      set(FT_DISABLE_HARFBUZZ  ON  CACHE BOOL "" FORCE)
      set(FT_REQUIRE_ZLIB      OFF CACHE BOOL "" FORCE)
      set(FT_REQUIRE_BZIP2     OFF CACHE BOOL "" FORCE)
      set(FT_REQUIRE_PNG       OFF CACHE BOOL "" FORCE)
      set(FT_REQUIRE_BROTLI    OFF CACHE BOOL "" FORCE)
      set(FT_WITH_ZLIB         OFF CACHE BOOL "" FORCE)
      set(FT_WITH_BZIP2        OFF CACHE BOOL "" FORCE)
      set(FT_WITH_PNG          OFF CACHE BOOL "" FORCE)
      set(FT_WITH_BROTLI       OFF CACHE BOOL "" FORCE)
      set(BUILD_TESTING        OFF CACHE BOOL "" FORCE)
      set(FT_DISABLE_DOCS      ON  CACHE BOOL "" FORCE)

      FetchContent_MakeAvailable(freetype_src)

      # Upstream defines the target 'freetype'. Create a consistent alias.
      if(TARGET freetype AND NOT TARGET Freetype::Freetype)
        add_library(Freetype::Freetype ALIAS freetype)
      endif()
    endif()

    if(NOT TARGET Freetype::Freetype)
      message(FATAL_ERROR "FreeType not found and fallback did not create target Freetype::Freetype")
    endif()
  endif()
endmacro()

macro(import_spdlog)
    if(NOT TARGET spdlog)  # Guard to prevent multiple inclusion
        FetchContent_Declare(
          spdlog
          GIT_REPOSITORY https://github.com/gabime/spdlog.git
          GIT_TAG v1.14.1
        )

        FetchContent_MakeAvailable(spdlog)

        include_directories(${spdlog_SOURCE_DIR}/include)
    endif()
endmacro()

# Macro to import all dependencies
macro(importDependencies)
    import_glfw()
    import_glew()
    import_glm()
    import_freetype()
    import_spdlog()
    import_json()
endmacro()