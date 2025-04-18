# /*******************************************************************/
# /*    NAME: Oscar Viquez                                           */
# /*    ORG:  Acbotics Research, LLC                                 */
# /*    FILE: CMakeLists.txt                                         */
# /*    DATE: Apr 4th 2024                                           */
# /*                                                                 */
# /*    For help, contact us at: support@acbotics.com                */
# /*******************************************************************/

if(${BUILD_APPS})

    add_executable (run_logger
        csv_logger.cpp
        ${SRC_COMMON}
        )

    install(TARGETS
        run_logger
        CONFIGURATIONS Release
        )

endif()
