# CMake script for hxo-loader
cmake_minimum_required(VERSION 3.20)

# project config
project(hxo-loader VERSION 1.2.0)

# configure the source files
set(SOURCE_FILES "src/hxo.c" "src/loader.c" "src/utils.c")

# set the version string
if(ANDROID)
    set(VERSION_STR "${PROJECT_VERSION}-android")
    add_definitions("-D__ANDROID__")
elseif(LINUX)
    set(VERSION_STR "${PROJECT_VERSION}-linux")
else()
    # for other systems
    set(VERSION_STR "${PROJECT_VERSION}")
endif()

# add the version string
add_definitions("-DVER_STR=\"${VERSION_STR}\"")
# build the thing
add_library(hxo SHARED ${SOURCE_FILES})
