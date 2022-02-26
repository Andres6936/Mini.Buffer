CMAKE_MINIMUM_REQUIRED(VERSION 3.7)

SET(PROJECT_NAME MiniFB)
PROJECT(${PROJECT_NAME})

MESSAGE(STATUS "Processing " ${PROJECT_NAME})

# Detect iOS
#--------------------------------------
IF (NOT DEFINED IOS)
    IF (DEFINED CMAKE_SYSTEM_NAME)
        STRING(TOLOWER CMAKE_SYSTEM_NAME CMAKE_SYSTEM_NAME_LOWER)
        IF (CMAKE_SYSTEM_NAME_LOWER STREQUAL "ios")
            SET(IOS true)
        ENDIF ()
    ENDIF ()
ENDIF ()

# Sources
#--------------------------------------
SET(SrcLib
        include/MiniFB.h
        include/MiniFB_cpp.h
        include/MiniFB_enums.h

        src/MiniFB_common.c
        src/MiniFB_cpp.cpp
        src/MiniFB_internal.c
        src/MiniFB_internal.h
        src/MiniFB_timer.c
        src/WindowData.h
        )

#--
SET(SrcWindows
        src/windows/WinMiniFB.c
        src/windows/WindowData_Win.h
        )

#--
SET(SrcMacOSX
        src/macosx/MacMiniFB.m
        src/macosx/OSXWindow.h
        src/macosx/OSXWindow.m
        src/macosx/OSXView.h
        src/macosx/OSXView.m
        src/macosx/OSXViewDelegate.h
        src/macosx/OSXViewDelegate.m
        src/macosx/WindowData_OSX.h
        )

#--
SET(SrcIOS
        src/ios/WindowData_IOS.h
        src/ios/iOSMiniFB.m
        src/ios/iOSView.h
        src/ios/iOSView.m
        src/ios/iOSViewController.h
        src/ios/iOSViewController.m
        src/ios/iOSViewDelegate.h
        src/ios/iOSViewDelegate.m
        include/MiniFB_ios.h
        )

#--
SET(SrcWayland
        src/wayland/WaylandMiniFB.c
        src/wayland/WindowData_Way.h
        src/MiniFB_linux.c
        )

#--
SET(SrcX11
        src/x11/X11MiniFB.c
        src/x11/WindowData_X11.h
        src/MiniFB_linux.c
        )

SET(SrcGL
        src/gl/MiniFB_GL.h
        src/gl/MiniFB_GL.c
        )

# Avoid RelWithDebInfo and MinSizeRel
#--------------------------------------
SET(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)

# Define Release by default
#--------------------------------------
IF (NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE "Release")
    MESSAGE(STATUS "Build type not specified: Use Release by default")
ENDIF (NOT CMAKE_BUILD_TYPE)

# Set features
#--------------------------------------
SET(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_CXX_EXTENSIONS OFF)

#--------------------------------------
SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)

# Code generation options
#--------------------------------------
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

# Set GCC/Clang flags
#--------------------------------------
IF (NOT MSVC)
    # Avoid default flag values
    #--------------------------------------
    SET(CMAKE_C_FLAGS "")
    SET(CMAKE_C_FLAGS_DEBUG "")
    SET(CMAKE_C_FLAGS_RELEASE "")

    SET(CMAKE_CXX_FLAGS "")
    SET(CMAKE_CXX_FLAGS_DEBUG "")
    SET(CMAKE_CXX_FLAGS_RELEASE "")

    SET(CMAKE_OBJC_FLAGS "")
    SET(CMAKE_OBJC_FLAGS_DEBUG "")
    SET(CMAKE_OBJC_FLAGS_RELEASE "")

    SET(CMAKE_OBJCXX_FLAGS "")
    SET(CMAKE_OBJCXX_FLAGS_DEBUG "")
    SET(CMAKE_OBJCXX_FLAGS_RELEASE "")

    # Set our flags
    #--------------------------------------
    ADD_COMPILE_OPTIONS("$<$<CONFIG:Debug>:-g>")
    ADD_COMPILE_OPTIONS("$<IF:$<CONFIG:Debug>,-O0,-O2>")
    ADD_COMPILE_OPTIONS(-Wall -Wextra)
    ADD_COMPILE_OPTIONS(-Wno-switch -Wno-unused-function -Wno-unused-parameter -Wno-implicit-fallthrough)

    IF (NOT APPLE)
        ADD_COMPILE_OPTIONS(-Wno-cast-function-type)
    ENDIF ()
ELSE ()
    # Security check
    ADD_COMPILE_OPTIONS(/GS)
    # Function level linking
    ADD_COMPILE_OPTIONS(/Gy)
    # Exceptions
    ADD_COMPILE_OPTIONS(/EHsc)
    IF (MSVC_VERSION GREATER_EQUAL 1900)
        # SDL checks 2015+
        ADD_COMPILE_OPTIONS(/sdl)
    ENDIF ()
    IF (MSVC_VERSION LESS_EQUAL 1920)
        # Enable Minimal Rebuild (required for Edit and Continue) (deprecated)
        ADD_COMPILE_OPTIONS(/Gm)
    ENDIF ()
    ADD_COMPILE_OPTIONS(/fp:fast)
    # Runtime library
    ADD_COMPILE_OPTIONS("$<IF:$<CONFIG:Debug>,/MDd,/MD>")
    # Program database for edit and continue
    ADD_COMPILE_OPTIONS("$<IF:$<CONFIG:Debug>,/ZI,/Zi>")
    # Optimizations
    ADD_COMPILE_OPTIONS("$<IF:$<CONFIG:Debug>,/Od,/O2>")
    # Inline function expansion
    ADD_COMPILE_OPTIONS("$<IF:$<CONFIG:Debug>,/Ob0,/Ob2>")
    # Basic runtime checks
    ADD_COMPILE_OPTIONS("$<$<CONFIG:Debug>:/RTC1>")
    # Force Visual Studio to actualize __cplusplus version macro
    ADD_COMPILE_OPTIONS(/Zc:__cplusplus)

ENDIF ()

IF (CMAKE_BUILD_TYPE STREQUAL "Debug")
    ADD_DEFINITIONS(-D_DEBUG)
    ADD_DEFINITIONS(-DDEBUG)
ENDIF ()

# Set compiler/platform specific flags and dependencies
#--------------------------------------
IF (WIN32)

    IF (MSVC)
        ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
    ENDIF ()
    ADD_DEFINITIONS(-D_WIN32_WINNT=0x0601)  # Windows 7 (we are in 2020)

    IF (USE_OPENGL_API)
        LIST(APPEND SrcLib ${SrcGL})

        ADD_DEFINITIONS(-DUSE_OPENGL_API)
    ENDIF ()

    LIST(APPEND SrcLib ${SrcWindows})

ELSEIF (IOS)

    LIST(APPEND SrcLib ${SrcIOS})

ELSEIF (APPLE)

    IF (USE_METAL_API)
        ADD_DEFINITIONS(-DUSE_METAL_API)
    ENDIF ()

    IF (USE_INVERTED_Y_ON_MACOS)
        ADD_DEFINITIONS(-DUSE_INVERTED_Y_ON_MACOS)
    ENDIF ()

    LIST(APPEND SrcLib ${SrcMacOSX})

ELSEIF (UNIX)

    IF (USE_WAYLAND_API)
        LIST(APPEND SrcLib ${SrcWayland})
    ELSE ()
        IF (USE_OPENGL_API)
            LIST(APPEND SrcLib ${SrcGL})

            ADD_DEFINITIONS(-DUSE_OPENGL_API)
        ENDIF ()
        LIST(APPEND SrcLib ${SrcX11})
    ENDIF ()

ENDIF ()

# Library
#--------------------------------------
ADD_LIBRARY(minifb STATIC
        ${SrcLib}
        )

# Link
#--------------------------------------
IF (APPLE)

    IF (IOS)
        TARGET_LINK_LIBRARIES(minifb
                "-framework UIKit"
                "-framework QuartzCore"
                "-framework Metal"
                "-framework MetalKit"
                )
    ELSE ()
        TARGET_LINK_LIBRARIES(minifb
                "-framework Cocoa"
                "-framework QuartzCore"
                "-framework Metal"
                "-framework MetalKit"
                )
    ENDIF ()

ELSEIF (UNIX)

    IF (USE_WAYLAND_API)
        TARGET_LINK_LIBRARIES(minifb
                "-lwayland-client"
                "-lwayland-cursor"
                )
    ELSE ()
        TARGET_LINK_LIBRARIES(minifb
                "-lX11"
                #"-lxkbcommon"
                #"-lXrandr" DPI NOT WORKING
                )
        IF (USE_OPENGL_API)
            TARGET_LINK_LIBRARIES(minifb
                    "-lGL"
                    )
        ENDIF ()
    ENDIF ()

ELSEIF (WIN32)

    IF (USE_OPENGL_API)
        TARGET_LINK_LIBRARIES(minifb
                "Opengl32.lib"
                )
    ENDIF ()

    TARGET_LINK_LIBRARIES(minifb
            "winmm.lib"
            )

ENDIF ()

# For all projects
#--------------------------------------
TARGET_INCLUDE_DIRECTORIES(minifb PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
TARGET_INCLUDE_DIRECTORIES(minifb PRIVATE src)

LINK_LIBRARIES(minifb)

# Examples
#--------------------------------------
IF (MINIFB_BUILD_EXAMPLES)
    IF (NOT IOS)

        ADD_EXECUTABLE(noise
                tests/noise.c
                )

        ADD_EXECUTABLE(input_events
                tests/input_events.c
                )

        ADD_EXECUTABLE(input_events_cpp
                tests/input_events_cpp.cpp
                )

        ADD_EXECUTABLE(multiple_windows
                tests/multiple_windows.c
                )

        ADD_EXECUTABLE(hidpi
                tests/hidpi.c
                )

    ELSE ()

        ADD_EXECUTABLE(noise
                tests/ios/main.m
                tests/ios/AppDelegate.h
                tests/ios/AppDelegate.m
                )

        SET(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Set CMake deployment target" ${FORCE_CACHE})

        TARGET_INCLUDE_DIRECTORIES(noise PRIVATE src)
        TARGET_INCLUDE_DIRECTORIES(noise PRIVATE src/ios)

        ADD_DEFINITIONS(-DTVOS_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET})

    ENDIF ()

ENDIF ()

# Organize IDE Folders
#--------------------------------------
SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)
SET_PROPERTY(TARGET minifb PROPERTY FOLDER "Libs")

IF (MINIFB_BUILD_EXAMPLES)
    SET_PROPERTY(TARGET noise PROPERTY FOLDER "Tests")
    SET_PROPERTY(TARGET input_events PROPERTY FOLDER "Tests")
    SET_PROPERTY(TARGET input_events_cpp PROPERTY FOLDER "Tests")
    SET_PROPERTY(TARGET multiple_windows PROPERTY FOLDER "Tests")
    SET_PROPERTY(TARGET hidpi PROPERTY FOLDER "Tests")
ENDIF ()

MESSAGE(STATUS "Done " ${PROJECT_NAME})


# Content Pack Manager (Wrapper over FetchContent)
INCLUDE(CMakeModules/CPM.cmake)

# In this case, the name of the NAME variable is important.
# because it will be used to determine the path where the
# source code of the project is located.
CPMADDPACKAGE(
        NAME SPNG
        GITHUB_REPOSITORY Andres6936/SPNG
        VERSION 2021.2.26
)

ADD_EXECUTABLE(MiniBuffer.PNG.App tests/PNG/App.cpp)
TARGET_INCLUDE_DIRECTORIES(MiniBuffer.PNG.App PRIVATE src)
TARGET_INCLUDE_DIRECTORIES(MiniBuffer.PNG.App PRIVATE src/ios)
TARGET_LINK_LIBRARIES(MiniBuffer.PNG.App PRIVATE Spng::Framework)

# Reference: https://www.py4u.net/discuss/2364298
ADD_CUSTOM_COMMAND(TARGET MiniBuffer.PNG.App POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${SPNG_BINARY_DIR}
        $<TARGET_FILE_DIR:MiniBuffer.PNG.App>)