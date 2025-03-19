"""
Acbotics Research, LLC
Author: Sam Fladung
Created: Apr 8, 2022
License: MIT
For help, contact support@acbotics.com
"""

import logging
import struct
import sys
from collections import namedtuple

import numpy as np

from ..data_containers.data_container_constant_rate import (
    DataContainer_Constant_Rate,
)

logger = logging.getLogger(__name__)


class UDP_Data_Protocol:
    def __init__(self):
        self.VERSION_MAJOR_IND = 2
        self.VERSION_MINOR_IND = 3
        self.version_major = 0
        self.version_minor = 1
        # Add scale field

    def decode_header(self, data):
        if len(data) < 4:
            logger.info("packet too short. " + repr(data))
            return None
        if not data[0] == ord("A") or not data[1] == ord("C"):
            logger.info("ignoring unrecognized packet header: " + repr(data[0:2]))
            return None
        # extract protocol version
        version_major = data[self.VERSION_MAJOR_IND]
        # version_minor = data[self.VERSION_MINOR_IND]

        # eventually use version number to get proper format
        header_fmt, header_data = self.get_header_info(version_major)
        header_length_b = struct.calcsize(header_fmt)
        header = header_data._make(struct.unpack(header_fmt, data[0:header_length_b]))

        return header

    def decode_data(self, data, header):
        header_length_b = struct.calcsize(self.get_header_info(header.VER_MAJ)[0])
        d = data[header_length_b:]
        data_array = np.frombuffer(d, dtype=np.int16).reshape(-1, header.NUM_CHANNELS).T
        return data_array

    def decode(self, data):
        header = self.decode_header(data)
        d = self.decode_data(data, header)
        dc = DataContainer_Constant_Rate(
            data=d,
            sample_rate=header.SAMPLE_RATE,
            start_time=np.datetime64(header.START_TIME, "ns"),
            frame_count=header.PACKET_NUM,
        )
        return dc

    def get_header_info(self, version):
        header_fmt1 = "!ccBBcBBIIqIdI"
        header_fmt3 = "!ccBBcBBIfqIdI"
        header_fmt4 = "!ccBBcBBIfqQIdI"

        header_data1 = namedtuple(
            "header_data",
            "ID1 ID2 VER_MAJ VER_MIN ENDIAN NUM_CHANNELS DATA_SIZE NUM_VALUES "
            "SAMPLE_RATE START_TIME ADC_COUNT SCALE PACKET_NUM",
        )

        header_data4 = namedtuple(
            "header_data",
            "ID1 ID2 VER_MAJ VER_MIN ENDIAN NUM_CHANNELS DATA_SIZE NUM_VALUES "
            "SAMPLE_RATE START_TIME TICK_TIME ADC_COUNT SCALE PACKET_NUM",
        )

        if version > 3:
            return header_fmt4, header_data4
        elif version > 2:
            return header_fmt3, header_data1
        else:
            return header_fmt1, header_data1

    def encode(self, data_array, sample_rate, start_time, scale, packet_number):
        frame_start_time = start_time.astype(">i8")

        data_endian = data_array.dtype.byteorder
        if data_endian == "=":  # system order:
            sys_endian = sys.byteorder
            if sys_endian == "little":
                data_endian = "<"
            else:
                data_endian = ">"

        header = struct.pack(
            self.header_fmt4,
            b"A",
            b"C",
            self.version_major,
            self.version_minor,
            data_endian.encode(),
            data_array.shape[0],
            data_array.dtype.itemsize,
            data_array.size,
            sample_rate,
            frame_start_time,
            0,
            0,
            scale,
            packet_number,
        )

        data_to_send = header + data_array.T.tobytes()
        return data_to_send
