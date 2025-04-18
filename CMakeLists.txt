# /*******************************************************************/
# /*    NAME: Oscar Viquez                                           */
# /*    ORG:  Acbotics Research, LLC                                 */
# /*    FILE: CMakeLists.txt                                         */
# /*    DATE: Apr 4th 2024                                           */
# /*                                                                 */
# /*    For help, contact us at: support@acbotics.com                */
# /*******************************************************************/

cmake_minimum_required(VERSION 3.16)

project(acbotics_interface VERSION 0.1.0)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(DO_SYSTEM_INSTALL true)
else()
    # For an opinionated setup, uncomment the next line:
    # set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/_install)
    set(DO_SYSTEM_INSTALL false)
endif()

message("** System Install: " ${DO_SYSTEM_INSTALL})
message("** Will install to " ${CMAKE_INSTALL_PREFIX})

find_package(glog 0.4.0 REQUIRED)
find_library(sndfile sndfile REQUIRED)

link_libraries(glog::glog sndfile gps)

message("-- glog v" ${glog_VERSION_MAJOR} "." ${glog_VERSION_MINOR} "." ${glog_VERSION_PATCH})

if("${glog_VERSION_MINOR}" LESS 6)
    add_compile_definitions(SKIP_GLOG_INIT_CHECK)
endif()

set(CMAKE_C_FLAGS "-flto=auto")
set(CMAKE_CXX_FLAGS "-flto=auto")

add_subdirectory(src)
