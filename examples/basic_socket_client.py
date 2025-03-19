import socket
import struct

import acbotics_interface.protocols as dprot

# Configure a socket to accept UDP data packets
# Example shown receives UDP multicast
sock_aco = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
sock_aco.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

sock_aco.bind(("", 9760))  # default for acoustic (ACO) data stream
# sock_aco.bind(("", 9770)) # default for non-acoustic sensor (SENS) data stream

# add multicast group membership to the host interface identified in mreq to receive
# UDP data; host is the machine running this program and the IP used should match
# one assigned to the interface on which data is expected (i.e. eth, wifi, loopback)
group = socket.inet_aton("224.1.1.1")
mreq = struct.pack("4s4s", group, socket.inet_aton("192.168.1.115"))
sock_aco.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)

while True:
    msg = sock_aco.recv(65535)

    if msg[:4] == b"ACB2":
        handler = dprot.UDP_Beamform_2D_Protocol()
    elif msg[:4] in [b"ACBR", b"ACBC"]:
        handler = dprot.UDP_Beamform_Raw_Protocol()
    elif msg[:2] == b"AC":
        handler = dprot.UDP_Data_Protocol()
    else:
        print(msg[8:10])
        continue

    header = handler.decode_header(msg)
    data = handler.decode(msg).data

    if msg[:2] == b"AC":
        # Convert int16 samples to voltage
        # > assuming gain set for +/- 2.5V range; adjust otherwise
        data = data * 2.5 / 2**15

    print(header)
    print(f"data shape is : {data.shape}")
    print()
