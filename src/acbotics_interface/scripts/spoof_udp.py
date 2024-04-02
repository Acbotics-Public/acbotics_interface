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

from acbotics_interface.protocols.udp_data_protocol import UDP_Data_Protocol


def spoof_raw_data():
    # Handle arguments
    parser = argparse.ArgumentParser(
        prog="Acbotics Spoofer : Acoustics",
        description="Spoof data for a hydrophone array",
        epilog="Written by Acbotics Research",
    )

    parser.add_argument(
        "--udp_port",
        type=int,
        default=5000,
        help="UDP port to send data to",
    )
    parser.add_argument(
        "--udp_ip",
        type=str,
        default="localhost",
        help="UDP IP to send data to",
    )
    parser.add_argument(
        "--freq",
        type=float,
        default=None,
        help="Sample tone frequency",
    )

    args = parser.parse_args()

    print(f"Will send data to {args.udp_ip}:{args.udp_port}")
    if args.freq:
        print(f"Sample tone frequency is {args.freq}")

    bot = UDP_Data_Protocol()

    Fs = 50000
    num_ch = 8
    step_size = 32
    delay = step_size / Fs

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

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
        sock.sendto(msg, (args.udp_ip, args.udp_port))

        pkt_num += 1
        time.sleep(delay)
