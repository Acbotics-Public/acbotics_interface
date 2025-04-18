# /*******************************************************************/
# /*    NAME: Oscar Viquez                                           */
# /*    ORG:  Acbotics Research, LLC                                 */
# /*    FILE: CMakeLists.txt                                         */
# /*    DATE: Apr 4th 2024                                           */
# /*                                                                 */
# /*    For help, contact us at: support@acbotics.com                */
# /*******************************************************************/

if(${BUILD_TESTS})

    add_executable(run_tests
        run_tests.cpp
        tests.cpp
        ${SRC_COMMON}
        )

    install(TARGETS
        run_tests
        CONFIGURATIONS Release
        )

endif()


