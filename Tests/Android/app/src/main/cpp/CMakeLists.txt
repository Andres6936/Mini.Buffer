CMAKE_MINIMUM_REQUIRED(VERSION 3.18.1)
PROJECT(noise)

# Set our flags
#--------------------------------------
#add_compile_options("$<$<CONFIG:Debug>:-g>")
#add_compile_options("$<IF:$<CONFIG:Debug>,-O0,-O2>")
#if(APPLE)
#    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic -Wno-switch -Wno-unused-function -Wno-implicit-fallthrough")
#else()
#    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic -Wno-switch -Wno-unused-function -Wno-implicit-fallthrough -Wno-cast-function-type")
#endif()
#set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS}")
#set(CMAKE_OBJC_FLAGS "${CMAKE_C_FLAGS}")
#set(CMAKE_OBJCXX_FLAGS "${CMAKE_CXX_FLAGS}")

# build native_app_glue as a static lib
ADD_LIBRARY(native_app_glue STATIC
        ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c
        )

ADD_LIBRARY(minifb STATIC
        ../../../../../../Include/MiniBF/MiniFB.h
        ../../../../../../Include/MiniBF/MiniFB_cpp.h
        ../../../../../../Include/MiniBF/MiniFB_enums.h

        ${CMAKE_CURRENT_LIST_DIR}/../../../../../../Source/MiniFB_common.c
        ${CMAKE_CURRENT_LIST_DIR}/../../../../../../Source/MiniFB_cpp.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../../../../../../Source/MiniFB_internal.c
        ../../../../../../Include/MiniBF/Internal/MiniFB_internal.h
        ${CMAKE_CURRENT_LIST_DIR}/../../../../../../Source/MiniFB_timer.c
        ${CMAKE_CURRENT_LIST_DIR}/../../../../../../Source/MiniFB_linux.c
        ../../../../../../Include/MiniBF/Internal/WindowData.h

        ${CMAKE_CURRENT_LIST_DIR}/../../../../../../Source/Android/AndroidMiniFB.c
        ../../../../../../Include/MiniBF/Internal/Android/WindowData_Android.h
        )

TARGET_INCLUDE_DIRECTORIES(minifb PRIVATE
        ${ANDROID_NDK}/sources/android/native_app_glue
        ${CMAKE_CURRENT_LIST_DIR}/../../../../../../include
        ${CMAKE_CURRENT_LIST_DIR}/../../../../../../Source
        )

# now build app's shared lib
ADD_LIBRARY(noise SHARED
        noise.c
        )

TARGET_INCLUDE_DIRECTORIES(noise PRIVATE
        ${ANDROID_NDK}/sources/android/native_app_glue
        ${CMAKE_CURRENT_LIST_DIR}/../../../../../../include
        )

# Export ANativeActivity_onCreate(),
# Refer to: https://github.com/android-ndk/ndk/issues/381.
SET(CMAKE_SHARED_LINKER_FLAGS
        "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate"
        )

# add lib dependencies
TARGET_LINK_LIBRARIES(noise
        android
        minifb
        native_app_glue
        log
        m
        )
