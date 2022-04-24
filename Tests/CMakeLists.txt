IF (MINIFB_BUILD_EXAMPLES)
    IF (NOT iOS)

        ADD_EXECUTABLE(noise
                noise.c
                )

        ADD_EXECUTABLE(input_events
                input_events.c
                )

        ADD_EXECUTABLE(input_events_cpp
                input_events_cpp.cpp
                )

        ADD_EXECUTABLE(multiple_windows
                multiple_windows.c
                )

        ADD_EXECUTABLE(hidpi
                hidpi.c
                )

    ELSE ()

        ADD_EXECUTABLE(noise
                iOS/main.m
                iOS/AppDelegate.h
                iOS/AppDelegate.m
                )

        SET(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Set CMake deployment target" ${FORCE_CACHE})

        TARGET_INCLUDE_DIRECTORIES(noise PRIVATE ${MINI_BF_SOURCE_DIR}/Source)
        TARGET_INCLUDE_DIRECTORIES(noise PRIVATE ${MINI_BF_SOURCE_DIR}/Source/iOS)

        ADD_DEFINITIONS(-DTVOS_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET})

    ENDIF ()

ENDIF ()


# Content Pack Manager (Wrapper over FetchContent)
INCLUDE(../CMakeModules/CPM.cmake)

# In this case, the name of the NAME variable is important.
# because it will be used to determine the path where the
# source code of the project is located.
CPMADDPACKAGE(
        NAME SPNG
        GITHUB_REPOSITORY Andres6936/SPNG
        VERSION 2021.4.16
)

ADD_EXECUTABLE(MiniBuffer.PNG.App PNG/App.cpp)
TARGET_INCLUDE_DIRECTORIES(MiniBuffer.PNG.App PRIVATE ${MINI_BF_SOURCE_DIR}/Source)
TARGET_INCLUDE_DIRECTORIES(MiniBuffer.PNG.App PRIVATE ${MINI_BF_SOURCE_DIR}/Source/iOS)
TARGET_LINK_LIBRARIES(MiniBuffer.PNG.App PRIVATE minifb Spng::Framework)

# Reference: https://www.py4u.net/discuss/2364298
ADD_CUSTOM_COMMAND(TARGET MiniBuffer.PNG.App POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        $<TARGET_FILE_DIR:Spng::Framework>
        $<TARGET_FILE_DIR:MiniBuffer.PNG.App>)


ADD_CUSTOM_COMMAND(TARGET MiniBuffer.PNG.App POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        $<TARGET_FILE_DIR:minifb>
        $<TARGET_FILE_DIR:MiniBuffer.PNG.App>)