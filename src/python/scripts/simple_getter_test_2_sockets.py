import acsense_udp

import numpy as np
import time


if __name__ == "__main__":

    acsense_udp.init_logger()

    hh = acsense_udp.InterfaceHelper()
    hh.add_socket(True, "192.168.1.115", 9760, "224.1.1.1")
    hh.add_socket(True, "192.168.1.115", 9766, "224.1.1.1")

    hh.enable_aco_thread(True)
    # hh.enable_pts_logging_csv(True)
    hh.run_threads()

    pts_init = False
    while True:
        tstart = time.time_ns()

        tt = time.time_ns()
        aa = np.array(hh.get_acoustic_data())
        pts_new = hh.get_pts_data()

        if pts_new.pressure_mbar > 0:
            pts = pts_new
            pts_init = True

        print(f"Aco. data shape : {aa.shape}")
        if pts_init:
            print(f"Pressure        : {pts.pressure_mbar:8.2f} mbar")
            print(f"Temperature     : {pts.temperature_c:8.2f} C")

        tsleep = np.max([0, 1 / 50 - (time.time_ns() - tstart) / 1e9])
        print()

        time.sleep(tsleep)
