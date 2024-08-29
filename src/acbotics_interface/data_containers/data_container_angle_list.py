"""
Acbotics Research, LLC
Author: Sam Fladung
Created: Mar 7, 2022
License: MIT
For help, contact support@acbotics.com
"""

import icontract
import numpy as np
import logging

from .data_container import DataContainer

logger = logging.getLogger(__name__)


class DataContainer_Angle_List(DataContainer):
    @icontract.require(
        lambda start_time: isinstance(start_time, np.datetime64),
        "start_time must be datetime64",
    )
    def __init__(self, data, start_time):
        if isinstance(data, list):
            logger.debug("converting list to array")
            data = np.array(data)
        self.data = data
        self.start_time = start_time

    def get_timestamps(self):
        return [self.start_time]

    def is_constant_rate(self):
        return False

    def get_start_time(self):
        return self.start_time

    def get_timestamped_data(self):
        return (self.start_time, self.data)

    def get_end_time(self):
        """Returns the time of the last sample"""
        return self.start_time

    def get_angles(self):
        return self.data
