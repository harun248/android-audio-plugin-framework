# Sets the minimum version of CMake required to build your native library.
# This ensures that a certain set of CMake features is available to
# your build.

cmake_minimum_required(VERSION 3.5.1)

if (NOT ANDROID)

find_package(PkgConfig REQUIRED)
find_package(Protobuf REQUIRED)
pkg_check_modules(PROTOBUF REQUIRED protobuf)
pkg_check_modules(GRPC REQUIRED grpc_unsecure)
pkg_check_modules(GRPCPP REQUIRED grpc++_unsecure)

endif (NOT ANDROID)

add_subdirectory ("androidaudioplugin")

if (NOT ANDROID)
	add_subdirectory ("samples")
	add_subdirectory ("samples/aapbarebonepluginsample")
	add_subdirectory ("tools/aap-list")
	add_subdirectory ("tools/aap-info")
endif (NOT ANDROID)

