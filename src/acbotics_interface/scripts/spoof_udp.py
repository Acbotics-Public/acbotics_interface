"""
Acbotics Research, LLC
Author: Oscar Viquez, ScD
Created: Oct 6, 2023
License: MIT
For help, contact support@acbotics.com
"""

import time
import numpy as np
import socket
import argparse
import logging

from ..protocols.udp_data_protocol import UDP_Data_Protocol

logger = logging.getLogger(__name__)


def spoof_raw_data():
    # Handle arguments
    parser = argparse.ArgumentParser(
        prog="Acbotics Spoofer : Acoustics",
        description="Spoof data for a hydrophone array",
        epilog="Written by Acbotics Research",
    )

    parser.add_argument(
        "--iface_ip",
        type=str,
        default="127.0.0.1",
        help="IP of interface to bind",
    )
    parser.add_argument(
        "--mcast_group",
        type=str,
        default="224.1.1.1",
        help="Multicast group to send data to",
    )
    parser.add_argument(
        "--port",
        type=int,
        default=9760,
        help="Port to send data to",
    )
    parser.add_argument(
        "--freq",
        type=float,
        default=None,
        help="Sample tone frequency",
    )

    args = parser.parse_args()

    logging.basicConfig(
        # format="[%(asctime)s] %(name)s.%(funcName)s() : \n\t%(message)s",
        format="[%(asctime)s] %(levelname)s: %(filename)s:L%(lineno)d : %(message)s",
        datefmt="%Y-%m-%d %H:%M:%S",
        # level=logging.DEBUG,
        level=logging.INFO,
        force=True,
    )

    logger.info(
        f"Will send data to {args.mcast_group}:{args.port} via interface with address {args.iface_ip}"
    )
    if args.freq:
        logger.info(f"Sample tone frequency is {args.freq}")

    bot = UDP_Data_Protocol()

    Fs = 50000
    num_ch = 8
    step_size = 64
    delay = step_size / Fs

    # sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    sock.bind((args.iface_ip, 9760))

    sock.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, 2)

    pkt_num = 0
    track_time = 0
    while True:
        samples = np.arange(track_time, track_time + step_size) / Fs
        arr = np.random.randint(
            -(2**14), 2**14, size=[num_ch, step_size], dtype=np.int16
        )
        if args.freq:
            arr[0, :] += (2**14 * np.sin(2 * np.pi * args.freq * samples)).astype(
                np.int16
            )

        track_time += step_size
        msg = bot.encode(arr, Fs, np.datetime64(time.time_ns(), "ns"), 1.0, pkt_num)
        sock.sendto(msg, (args.mcast_group, args.port))

        pkt_num += 1
        time.sleep(delay)
