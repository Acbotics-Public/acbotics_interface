# /*******************************************************************/
# /*    NAME: Oscar Viquez                                           */
# /*    ORG:  Acbotics Research, LLC                                 */
# /*    FILE: CMakeLists.txt                                         */
# /*    DATE: Apr 4th 2024                                           */
# /*                                                                 */
# /*    For help, contact us at: support@acbotics.com                */
# /*******************************************************************/

add_subdirectory(pybind11)

if (${DO_SYSTEM_INSTALL})
    # Use the absolute path from distutils to ensure the output
    # is installed for the active Python environment (venv or system)
    # e.g. <PATH-TO-VENV>/lib/pythonX.YY/site-packages
    execute_process(
        COMMAND "${PYTHON_EXECUTABLE}" -c "if True:
        from distutils import sysconfig as sc
        print(sc.get_python_lib(plat_specific=True))"
        OUTPUT_VARIABLE PYTHON_SITE
        OUTPUT_STRIP_TRAILING_WHITESPACE)
else()
    # The prefix='' argument removes the absolute path to the
    # root of the python environment in use (venv or system);
    # e.g. <PREFIX>/lib/pythonX.YY/site-packages becomes
    # <CMAKE_INSTALL_PREFIX>/lib/pythonX.YY/site-packages
    execute_process(
        COMMAND "${PYTHON_EXECUTABLE}" -c "if True:
        from distutils import sysconfig as sc
        print(sc.get_python_lib(prefix='', plat_specific=True))"
        OUTPUT_VARIABLE PYTHON_SITE
        OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()

message("** Python v" ${PYTHON_VERSION_MAJOR} "." ${PYTHON_VERSION_MINOR})
message("** Python package will install to : " ${PYTHON_SITE} )

# Embed version tag from git tag/commit/status
execute_process(
        COMMAND ${CMAKE_SOURCE_DIR}/get_version_tag.sh
        OUTPUT_VARIABLE BUILD_TAG
        OUTPUT_STRIP_TRAILING_WHITESPACE)
message("** Build version tag is : " ${BUILD_TAG})
add_definitions( -DBUILD_TAG="${BUILD_TAG}" )

file(GLOB SRC_PYBIND
    *.cpp
    )

pybind11_add_module(acbotics_interface_py
    ${SRC_PROTOCOLS}
    ${SRC_UTILS}
    ${SRC_TESTS}
    ${SRC_PYBIND})

set_target_properties(acbotics_interface_py PROPERTIES OUTPUT_NAME acbotics_interface)
target_link_libraries(acbotics_interface_py PRIVATE acbotics_interface glog::glog sndfile)


install(TARGETS
    acbotics_interface_py
    LIBRARY DESTINATION ${PYTHON_SITE}
    CONFIGURATIONS Release)
