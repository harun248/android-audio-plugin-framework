# Sets the minimum version of CMake required to build your native library.
# This ensures that a certain set of CMake features is available to
# your build.

cmake_minimum_required(VERSION 3.4.1)

link_directories(
        "../build/${CMAKE_ANDROID_ARCH_ABI}/"
)

# Specifies a library name, specifies whether the library is STATIC or
# SHARED, and provides relative paths to the source code. You can
# define multiple libraries by adding multiple add_library() commands,
# and CMake builds them for you. When you build your app, Gradle
# automatically packages shared libraries with your APK.

add_executable(aaphostsample
        "AAPSampleLV2Interop_native.cpp"
        "AAPSampleLV2Interop_desktop.cpp"
        )

target_include_directories(aaphostsample
        PRIVATE
        "../plugin-api/include"
        "../core/include"
        )

target_compile_options(aaphostsample
        PRIVATE
        -std=c++17
        -Wall
        -Wextra-semi
        -Wshadow
        )

target_link_libraries(aaphostsample lilv-0 androidaudioplugin)

