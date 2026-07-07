# /*******************************************************************/
# /*    NAME: Oscar Viquez                                           */
# /*    ORG:  Acbotics Research, LLC                                 */
# /*    FILE: CMakeLists.txt                                         */
# /*    DATE: Apr 4th 2024                                           */
# /*                                                                 */
# /*    For help, contact us at: support@acbotics.com                */
# /*******************************************************************/

cmake_minimum_required(VERSION 3.6)

project(acbotics_interface VERSION 0.1.0)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(DO_SYSTEM_INSTALL true)
else()
    # For an opinionated setup, uncomment the next line:
    # set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/_install)
    set(DO_SYSTEM_INSTALL false)
endif()

message("** System Install: " ${DO_SYSTEM_INSTALL})
message("** Will install to " ${CMAKE_INSTALL_PREFIX})

#find_package(glog 0.4.0 REQUIRED)
include(FetchContent)
FetchContent_Declare(
  gflags
  GIT_REPOSITORY https://github.com/gflags/gflags.git
  GIT_TAG        v2.2.2 # Stable, widely compatible version
)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(gflags)

FetchContent_Declare(
  glog
  GIT_REPOSITORY https://github.com/google/glog.git
  GIT_TAG        v0.7.1 # Or v0.4.0, depending on your exact requirement
)

# Force static linking for the wheel
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(WITH_GFLAGS ON CACHE BOOL "" FORCE) 

FetchContent_MakeAvailable(glog)

FetchContent_Declare(
  libsndfile
  GIT_REPOSITORY https://github.com/libsndfile/libsndfile.git
  GIT_TAG        1.2.2 # Use the latest stable release
)

# Force static linking for the wheel
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

# Disable utilities we don't need for a Python wheel
set(BUILD_PROGRAMS OFF CACHE BOOL "" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)

# CRITICAL: Disable external codecs if you only need WAV/AIFF/PCM.
# If you leave this ON, CMake will fail unless you also FetchContent FLAC, Ogg, etc.
set(ENABLE_EXTERNAL_LIBS OFF CACHE BOOL "" FORCE)
set(ENABLE_MPEG OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(libsndfile)

FetchContent_Declare(
  eigen
  GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
  GIT_TAG        3.4.0 # A highly stable and widely compatible version
)

# Optional: Disable building Eigen's internal tests to speed up the wheel build
set(EIGEN_BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(EIGEN_BUILD_DOC OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(eigen)

find_path(GPS_INCLUDE_DIR NAMES libgpsmm.h gps.h)

# Find the shared library (libgps.so)
find_library(GPS_LIBRARY NAMES gps)

if(NOT GPS_INCLUDE_DIR OR NOT GPS_LIBRARY)
    message(FATAL_ERROR "Could not find gpsd headers or library! Ensure gpsd-devel is installed.")
endif()

link_libraries(gflags glog::glog sndfile gps Eigen3::Eigen)

message("-- glog v" ${glog_VERSION_MAJOR} "." ${glog_VERSION_MINOR} "." ${glog_VERSION_PATCH})


if("${glog_VERSION_MINOR}" LESS 6)
    add_compile_definitions(SKIP_GLOG_INIT_CHECK)
endif()

set(CMAKE_C_FLAGS "-flto=auto")
set(CMAKE_CXX_FLAGS "-flto=auto")

add_subdirectory(src)
