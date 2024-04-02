# Acbotics UDP Interface

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)
![package testing](https://github.com/acbotics-public/acbotics_interface/actions/workflows/build-test.yml/badge.svg?branch=main)

This package provides the basic code for the Acbotics UDP Protocol.

## Installation

Make sure you have `setuptools` installed:

```bash
pip install -U setuptools
```

You can then install `acbotics_interface` locally using either of the following:

```bash
# Install in editable mode:
# This will link to the source code and use changes immediately
pip install -e .

# Install in standard mode:
# This will install a distribution copy in the user or system package collection;
# source code updates will not be available prior to reinstallation or package update 
pip install .
```

## Usage

### Module

This package provides an importable module for handling data packets transmitted by the AcSense.

```python

import socket
from acbotics_interface.protocols.udp_data_protocol import UDP_Data_Protocol

# Configure a socket to accept UDP data packets
# Example shown receives UDP multicast
sock_aco = socket.socket(
    socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP
)
sock_aco.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

sock_aco.bind(("", 9760))

group = socket.inet_aton("224.1.1.1")
mreq = struct.pack("4s4s", group, socket.inet_aton("192.168.1.115"))
sock_aco.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)

msg = sock_aco.recv(65535)

handler = UDP_Data_Protocol()
header = handler.decode_header(msg)
data = handler.decode(msg).data

# Convert int16 samples to voltage
# > assuming gain set for +/- 1V range; adjust otherwise
data /= 2**15 

```

### Developer tools

The package also provides a device emulator in the form of a basic acoustic data spoofer. This can be used to facilitate the development of data pipelines starting at the data client's network interface; UDP packets are formatted in accordance with the AcSense specifications. For usage information, install the package and then execute:

```bash
spoof-raw-data -h
```

The provided spoofer example uses point-to-point UDP, rather than transmitting multicast UDP.
