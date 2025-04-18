import acsense_udp

import numpy as np
import time


if __name__ == "__main__":

    acsense_udp.init_logger()

    hh = acsense_udp.InterfaceHelper()
    hh.add_socket(True, "192.168.1.115", 9760, "224.1.1.1")

    hh.enable_aco_thread(True)

    hh.run_threads()

    while True:
        tstart = time.time_ns()

        tt = time.time_ns()
        aa = np.array(hh.get_acoustic_data())

        print((time.time_ns() - tt) / 1e9)
        print(type(aa))
        print(aa.shape)
        print(aa[0, :])

        tsleep = np.max([0, 1 / 50 - (time.time_ns() - tstart) / 1e9])
        print(f"Sleepimng {tsleep}")
        time.sleep(tsleep)
        print()
