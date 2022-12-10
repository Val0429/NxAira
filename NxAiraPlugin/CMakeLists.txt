## Copyright 2018-present Network Optix, Inc. Licensed under MPL 2.0: www.mozilla.org/MPL/2.0/

cmake_minimum_required(VERSION 3.3.2)
project(NxAiraPlugin)

set(metadataSdkDir "C:/nxwitness-metadata_sdk-5.0.0.35136-universal/metadata_sdk" CACHE PATH "C:/nxwitness-metadata_sdk-5.0.0.35136-universal/metadata_sdk")
if(metadataSdkDir STREQUAL "")
    set(metadataSdkDir ${CMAKE_CURRENT_LIST_DIR}/../..) #< Assume building samples inside the SDK.
    if(NOT EXISTS ${metadataSdkDir}/src/nx/sdk OR NOT EXISTS ${metadataSdkDir}/nx_kit/src/nx/kit)
        message(FATAL_ERROR "Define metadataSdkDir cache variable to point to the unzipped SDK.")
    endif()
endif()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})

# Download OpenCV automatically using Conan
if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
   message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
   file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/master/conan.cmake"
       "${CMAKE_BINARY_DIR}/conan.cmake")
endif()
include(${CMAKE_BINARY_DIR}/conan.cmake)

if(WIN32)
    set(CONAN_VS_RUNTIME_MT_SETTING "compiler.runtime=MT")
endif()
set(OPENCV_OPTIONS
    opencv:parallel=False
    opencv:contrib=True
    opencv:contrib_freetype=False
    opencv:contrib_sfm=False
    opencv:with_jpeg=False 
    opencv:with_png=False
    opencv:with_tiff=False
    opencv:with_jpeg2000=False
    opencv:with_openexr=False
    opencv:with_eigen=False
    opencv:with_webp=False
    opencv:with_quirc=False
    opencv:with_cuda=False
    opencv:with_cublas=False
    opencv:dnn=True
    opencv:with_ffmpeg=False
)

if(UNIX)
    set(OPENCV_OPTIONS 
        ${OPENCV_OPTIONS}
        opencv:fPIC=True
        opencv:with_gtk=False
        opencv:with_cufft=False
        opencv:with_v4l=False
    )
endif()

conan_cmake_run(BUILD_TYPE "Release")
conan_cmake_configure(REQUIRES opencv/4.5.5
    GENERATORS cmake_find_package
    OPTIONS ${OPENCV_OPTIONS}
    SETTINGS ${CONAN_VS_RUNTIME_MT_SETTING}
)

conan_cmake_install(PATH_OR_REFERENCE .
    OPTIONS ${OPENCV_OPTIONS}
    SETTINGS ${CONAN_VS_RUNTIME_MT_SETTING}
    BUILD missing
)

find_package(OpenCV REQUIRED)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(UNIX)
    # In Linux, for the plugin .so library, set `rpath` to "$ORIGIN" and do not set `runpath`, thus
    # enabling the lookup of the dependencies in the plugin dir first.
    string(APPEND CMAKE_SHARED_LINKER_FLAGS " -Wl,--disable-new-dtags")
endif()
set(CMAKE_SKIP_BUILD_RPATH ON)
set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)
set(CMAKE_INSTALL_RPATH "$ORIGIN")

if(WIN32)
    string(APPEND CMAKE_CXX_FLAGS " /MP") #< Use all CPU cores by MSVC.
    # Do not create separate .pdb files for object files (workaround for mspdbsrv.exe bug).
    add_compile_options("/Z7")
endif()

if(WIN32)
    set(API_EXPORT_MACRO "__declspec(dllexport)")
else()
    set(API_EXPORT_MACRO "__attribute__((visibility(\"default\")))")
endif()

#--------------------------------------------------------------------------------------------------
# Define nx_kit lib, static.

set(nxKitLibraryType "STATIC" CACHE STRING "" FORCE)
set(nxKitWithTests "NO" CACHE STRING "" FORCE)

add_subdirectory(${metadataSdkDir}/nx_kit ${CMAKE_CURRENT_BINARY_DIR}/nx_kit)

#--------------------------------------------------------------------------------------------------
# Define nx_sdk lib, static, depends on nx_kit.

set(SDK_SRC_DIR ${metadataSdkDir}/src)
file(GLOB_RECURSE SDK_SRC CONFIGURE_DEPENDS ${SDK_SRC_DIR}/*)

add_library(nx_sdk STATIC ${SDK_SRC})
target_include_directories(nx_sdk PUBLIC ${SDK_SRC_DIR})
target_link_libraries(nx_sdk PRIVATE nx_kit)

target_compile_definitions(nx_sdk PRIVATE NX_PLUGIN_API=${API_EXPORT_MACRO}) #< for nxLibContext()

#--------------------------------------------------------------------------------------------------
# Define NxAiraPlugin lib, dynamic, depends on nx_kit and nx_sdk.

set(NXAIRA_PLUGIN_SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/src)
file(GLOB_RECURSE NXAIRA_PLUGIN_SRC CONFIGURE_DEPENDS
    ${NXAIRA_PLUGIN_SRC_DIR}/*)

add_library(NxAiraPlugin SHARED ${NXAIRA_PLUGIN_SRC})
target_include_directories(NxAiraPlugin PRIVATE ${NXAIRA_PLUGIN_SRC_DIR})
target_link_libraries(NxAiraPlugin PRIVATE nx_kit nx_sdk)

target_compile_definitions(NxAiraPlugin
    PRIVATE NX_PLUGIN_API=${API_EXPORT_MACRO}
)
