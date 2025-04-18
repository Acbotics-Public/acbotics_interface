/*******************************************************************/
/*    NAME: Oscar Viquez                                           */
/*    ORG:  Acbotics Research, LLC                                 */
/*    FILE: Types.h                                                */
/*    DATE: Feb 7th 2025                                           */
/*                                                                 */
/*    For help, contact us at: support@acbotics.com                */
/*******************************************************************/

#ifndef types_HEADER
#define types_HEADER

#include <unordered_map>

enum class QUEUE { UNKNOWN, ACO, FFT, CBF, GPS, PTS, IMU, EPT, RTC, BNO, BNR, DETECT };

enum class LOGGER { UNKNOWN, ACO_CSV, ACO_FLAC, ACO_WAV, GPS, PTS, IMU, EPT, RTC, BNO, BNR };

inline std::unordered_map<LOGGER, std::string> LOGGER_NAME{
    {LOGGER::ACO_CSV, "ACO_CSV"}, {LOGGER::ACO_FLAC, "ACO_FLAC"}, {LOGGER::ACO_WAV, "ACO_WAV"},
    {LOGGER::GPS, "GPS"},         {LOGGER::PTS, "PTS"},           {LOGGER::IMU, "IMU"},
    {LOGGER::EPT, "EPT"},         {LOGGER::RTC, "RTC"},           {LOGGER::BNO, "BNO"},
    {LOGGER::BNR, "BNR"}};

inline std::unordered_map<LOGGER, QUEUE> LOGGER_QUEUE{
    {LOGGER::ACO_CSV, QUEUE::ACO}, {LOGGER::ACO_FLAC, QUEUE::ACO}, {LOGGER::ACO_WAV, QUEUE::ACO},
    {LOGGER::GPS, QUEUE::GPS},     {LOGGER::PTS, QUEUE::PTS},      {LOGGER::IMU, QUEUE::IMU},
    {LOGGER::EPT, QUEUE::EPT},     {LOGGER::RTC, QUEUE::RTC},      {LOGGER::BNO, QUEUE::BNO},
    {LOGGER::BNR, QUEUE::BNR}};

#endif
