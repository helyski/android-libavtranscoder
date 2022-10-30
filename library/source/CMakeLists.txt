cmake_minimum_required(VERSION 3.10.2)

# 设置C++ 11
set(CMAKE_CXX_STANDARD 11)

# 待补充
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

# 待补充
set_property(SOURCE ${SOURCES} APPEND_STRING PROPERTY COMPILE_FLAGS " -mfpu=neon")
add_definitions("-DHAVE_NEON=1")

# 待补充
add_definitions(-DSOCKLEN_T=socklen_t)
add_definitions(-DNO_SSTREAM=1)
add_definitions(-DNEWLOCALE_NOT_USED)
#add_definitions(-D_LARGEFILE_SOURCE=1)
#add_definitions(-D_FILE_OFFSET_BITS=64)
add_definitions(-DBSD=1)
add_definitions(-DNO_CRYPTO=1)



# 设置头文件目录，逐级添加，如果没有添加根目录可能会失败
include_directories(
        ${CMAKE_SOURCE_DIR}/source
        ${CMAKE_SOURCE_DIR}/source/core
        ${CMAKE_SOURCE_DIR}/source/tools
        ${CMAKE_SOURCE_DIR}/source/ffmpeg/include
        ${CMAKE_SOURCE_DIR}/source/core/ffmpeg-wrappers
        ${CMAKE_SOURCE_DIR}/source/core/thread
        ${CMAKE_SOURCE_DIR}/source/core/android
        ${CMAKE_SOURCE_DIR}/source/libyuvmaster
)

# 设置源文件，单个文件用set，多个文件可以使用file
set(main_src transcode_jni.cpp)
file (GLOB_RECURSE core_src "core/*.c*" )
file (GLOB_RECURSE tool_src "tools/*.c*" )
file (GLOB_RECURSE ffmpeg_wrapper_src "core/ffmpeg-wrappers/*.c*" )
file (GLOB_RECURSE mediacodec_wrapper_src "core/android/*.c*" )


add_library( # Sets the name of the library.
        avtranscoder

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        ${main_src}
        ${core_src}
        ${tool_src}
        ${ffmpeg_wrapper_src}
        )

# 加系统库
find_library( # Sets the name of the path variable.
        log-lib
        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log )


# 加第三方动态库
add_library(libffmpeg SHARED IMPORTED)
add_library(libx265 SHARED IMPORTED)
add_library(libyuv SHARED IMPORTED)

# 根据架构选择对应的动态库文件
if(${ANDROID_ABI} STREQUAL "armeabi-v7a")
    set_target_properties(
            libffmpeg
            PROPERTIES IMPORTED_LOCATION
            ${CMAKE_SOURCE_DIR}/../app/libs/armeabi-v7a/libffmpeg.so
    )
    set_target_properties(
            libx265
            PROPERTIES IMPORTED_LOCATION
            ${CMAKE_SOURCE_DIR}/../app/libs/armeabi-v7a/libx265.so
    )
    set_target_properties(
            libyuv
            PROPERTIES IMPORTED_LOCATION
            ${CMAKE_SOURCE_DIR}/../app/libs/armeabi-v7a/libyuv.so
    )
endif(${ANDROID_ABI} STREQUAL "armeabi-v7a")

if(${ANDROID_ABI} STREQUAL "arm64-v8a")
    set_target_properties(
            libffmpeg
            PROPERTIES IMPORTED_LOCATION
            ${CMAKE_SOURCE_DIR}/../app/libs/arm64-v8a/libffmpeg.so
    )
    set_target_properties(
            libx265
            PROPERTIES IMPORTED_LOCATION
            ${CMAKE_SOURCE_DIR}/../app/libs/arm64-v8a/libx265.so
    )
    set_target_properties(
            libyuv
            PROPERTIES IMPORTED_LOCATION
            ${CMAKE_SOURCE_DIR}/../app/libs/arm64-v8a/libyuv.so
    )
endif(${ANDROID_ABI} STREQUAL "arm64-v8a")

# 设置目标库链接的第三方静态库

target_link_libraries(avtranscoder libffmpeg)
target_link_libraries(avtranscoder libx265)
target_link_libraries(avtranscoder libyuv)
target_link_libraries(avtranscoder jnigraphics)
target_link_libraries(avtranscoder OpenSLES)
target_link_libraries(avtranscoder ${log-lib})