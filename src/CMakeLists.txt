# Sources
#--------------------------------------
SET(SrcLib
        MiniFB_common.c
        MiniFB_cpp.cpp
        MiniFB_internal.c
        MiniFB_timer.c
        )

#--
SET(SrcWindows
        windows/WinMiniFB.c
        )

#--
SET(SrcMacOSX
        macosx/MacMiniFB.m
        macosx/OSXWindow.m
        macosx/OSXView.m
        macosx/OSXViewDelegate.m
        )

#--
SET(SrcIOS
        ios/iOSMiniFB.m
        ios/iOSView.m
        ios/iOSViewController.m
        ios/iOSViewDelegate.m
        )

#--
SET(SrcWayland
        wayland/WaylandMiniFB.c
        MiniFB_linux.c
        )

#--
SET(SrcX11
        x11/X11MiniFB.c
        MiniFB_linux.c
        )

SET(SrcGL
        gl/MiniFB_GL.c
        )


# Set features
#--------------------------------------
SET(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_CXX_EXTENSIONS OFF)


# Set GCC/Clang flags
#--------------------------------------
IF (MSVC)
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


TARGET_INCLUDE_DIRECTORIES(minifb PUBLIC $<BUILD_INTERFACE:${MINI_BF_SOURCE_DIR}/include/>)
TARGET_INCLUDE_DIRECTORIES(minifb PRIVATE $<BUILD_INTERFACE:${MINI_BF_SOURCE_DIR}/src/>)