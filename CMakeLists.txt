CMAKE_MINIMUM_REQUIRED(VERSION 3.14)

PROJECT(MINI_BF LANGUAGES C CXX VERSION 2022.04.16)

SET(MINI_BF_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
SET(MINI_BF_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})

# Detect iOS
IF (NOT DEFINED IOS)
    IF (DEFINED CMAKE_SYSTEM_NAME)
        STRING(TOLOWER CMAKE_SYSTEM_NAME CMAKE_SYSTEM_NAME_LOWER)
        IF (CMAKE_SYSTEM_NAME_LOWER STREQUAL "ios")
            SET(IOS true)
        ENDIF ()
    ENDIF ()
ENDIF ()


OPTION(MINIFB_BUILD_EXAMPLES "Build minifb example programs" TRUE)
OPTION(MINIFB_AVOID_CPP_HEADERS "Avoid including C++ Headers" FALSE)

IF (APPLE AND NOT IOS)
    OPTION(USE_METAL_API "Build the project using metal API code" ON)
    OPTION(USE_INVERTED_Y_ON_MACOS "Use default mouse position: (0, 0) at (left, down)" OFF)
ELSEIF (UNIX)
    OPTION(USE_WAYLAND_API "Build the project using wayland API code" OFF)
    IF (NOT USE_WAYLAND_API)
        OPTION(USE_OPENGL_API "Build the project using OpenGL API code" ON)
    ENDIF ()
ELSEIF (WIN32)
    OPTION(USE_OPENGL_API "Build the project using OpenGL API code" ON)
ENDIF ()


# Build the libraries and binaries for the main project
ADD_SUBDIRECTORY(Source/)
# Build binaries for Examples if the options is enable
ADD_SUBDIRECTORY(Tests/)
