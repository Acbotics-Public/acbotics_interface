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


class DataContainer_Beamformed_Output_2D(DataContainer):
    @icontract.require(
        lambda start_time: isinstance(start_time, np.datetime64),
        "start_time must be datetime64",
    )
    def __init__(
        self,
        data,
        thetas,
        phis,
        frequencies,
        start_time,
        array_x,
        array_y,
        array_z,
        window_length_s,
        sample_rate,
        element_mask,
        mode,
        weighting_type,
        xform_pitch,
        xform_roll,
        xform_yaw,
        element_weights,
    ):
        if isinstance(data, list):
            # logger.debug("converting list to array")
            data = np.array(data)
        self.data = data
        self.start_time = start_time
        self.thetas = thetas
        if isinstance(self.thetas, list):
            self.thetas = np.array(self.thetas, dtype=np.float64)
        elif self.thetas.dtype != np.float64:
            self.thetas = self.thetas.astype(np.float64)
        self.phis = phis
        if isinstance(self.phis, list):
            self.phis = np.array(self.phis, dtype=np.float64)
        elif self.phis.dtype != np.float64:
            self.phis = self.phis.astype(np.float64)

        self.frequencies = frequencies
        if isinstance(self.frequencies, list):
            self.frequencies = np.array(self.frequencies, dtype=np.float64)
        elif self.frequencies.dtype != np.float64:
            self.frequencies = self.frequencies.astype(np.float64)

        self.array_x = array_x
        if isinstance(self.array_x, list):
            self.array_x = np.array(self.array_x, dtype=np.float64)
        elif self.array_x.dtype != np.float64:
            self.array_x = self.array_x.astype(np.float64)

        self.array_y = array_y
        if isinstance(self.array_y, list):
            self.array_y = np.array(self.array_y, dtype=np.float64)
        elif self.array_y.dtype != np.float64:
            self.array_y = self.array_y.astype(np.float64)

        self.array_z = array_z
        if isinstance(self.array_z, list):
            self.array_z = np.array(self.array_z, dtype=np.float64)
        elif self.array_z.dtype != np.float64:
            self.array_z = self.array_z.astype(np.float64)

        self.window_length_s = window_length_s
        self.element_mask = element_mask
        if isinstance(self.element_mask, list):
            self.element_mask = np.array(self.element_mask, dtype=np.uint8)
        elif self.element_mask.dtype != np.uint8:
            self.element_mask = self.element_mask.astype(np.uint8)

        self.sample_rate = sample_rate
        self.mode = mode
        self.weighting_type = weighting_type
        self.xform_pitch = xform_pitch
        self.xform_roll = xform_roll
        self.xform_yaw = xform_yaw
        self.element_weights = element_weights
        if isinstance(self.element_weights, list):
            self.element_weights = np.array(self.element_weights, dtype=np.float64)
        elif self.element_weights.dtype != np.float64:
            self.element_weights = self.element_weights.astype(np.float64)
        logger.debug(f"Element mask : {self.element_mask}")
        logger.debug(f"Array X : {self.array_x}")
        assert self.element_mask.size == self.array_x.size
        assert self.element_weights.size == self.array_x.size
        assert self.array_y.size == self.array_x.size
        assert self.array_z.size == self.array_x.size

    def get_timestamps(self):
        return [
            self.start_time + i * 1e9 / self.sample_rate
            for i in range(self._calculate_data_length())
        ]

    def is_constant_rate(self):
        return False

    def get_start_time(self):
        return self.start_time

    def get_timestamped_data(self):
        return (self.start_time,)

    def get_end_time(self):
        """Returns the time of the last sample"""
        return self.start_time

    def get_thetas(self):
        return self.thetas

    def get_phis(self):
        return self.phis
