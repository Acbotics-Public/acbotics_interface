"""
Acbotics Research, LLC
Author: Sam Fladung
Created: Mar 7, 2022
License: MIT
For help, contact support@acbotics.com
"""

from abc import ABC, abstractmethod


class DataContainer(ABC):
    def __init__(self):
        pass

    @abstractmethod
    def get_timestamps(self):
        pass

    @abstractmethod
    def is_constant_rate(self):
        pass

    @abstractmethod
    def get_start_time(self):
        pass

    def get_timestamped_data(self):
        pass
