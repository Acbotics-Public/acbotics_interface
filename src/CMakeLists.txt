# /*******************************************************************/
# /*    NAME: Oscar Viquez                                           */
# /*    ORG:  Acbotics Research, LLC                                 */
# /*    FILE: CMakeLists.txt                                         */
# /*    DATE: Apr 4th 2024                                           */
# /*                                                                 */
# /*    For help, contact us at: support@acbotics.com                */
# /*******************************************************************/

include_directories(".")
include_directories("../submodules/pocketfft/")

file(GLOB SRC_PROTOCOLS
    udp_protocols/*.cpp
    ipc_protocols/*.cpp
    )

file(GLOB SRC_UTILS
    utils/*.cpp
    )

set(SRC_COMMON
    ${SRC_PROTOCOLS}
    ${SRC_UTILS}
    )

# Header Files
install(
    DIRECTORY ${CMAKE_SOURCE_DIR}/src/
    DESTINATION ${CMAKE_INSTALL_PREFIX}/include/acbotics_interface
    FILES_MATCHING PATTERN "*.h*"
    PATTERN "python" EXCLUDE
    PATTERN "apps" EXCLUDE
    PATTERN "tests.h" EXCLUDE
    )

# Python Bindings
add_subdirectory(python)

# Stand-alone Executables
add_subdirectory(apps)

# Library
add_library(acbotics_interface ${SRC_PROTOCOLS} ${SRC_UTILS})

install(TARGETS
    acbotics_interface
    CONFIGURATIONS Release
    )

# Tests (optional)
add_subdirectory(tests)
