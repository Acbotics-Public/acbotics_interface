import glob
import multiprocessing
import os
import time

import acsense_udp


# Your foo function
def test_single_thread(ss: acsense_udp.UdpSocketIn):
    # rr = acsense_udp.UdpSocketIn()
    qq = acsense_udp.QueueClient()
    ss.register_client(qq)
    ss.run_socket_main_thread()


def test_multithread(ss: acsense_udp.UdpSocketIn):
    # rr = acsense_udp.UdpSocketIn()
    qq = acsense_udp.QueueClient()
    ss.register_client(qq)
    ss.run_socket_thread()
    while True:
        print("Test tick -- main thread")
        time.sleep(1)


def test_logger_buffer(ss: acsense_udp.UdpSocketIn, sleep_time: float):
    hh = acsense_udp.InterfaceHelper()
    hh.add_socket(ss.use_mcast, ss.iface_ip, ss.port, ss.mcast_group)
    hh.enable_aco_fork(True)
    hh.enable_logger(acsense_udp.LOGGER.ACO_CSV, True)

    hh.run_threads()

    while True:
        aa = hh.get_acoustic_data()
        print("Test tick -- main thread")
        fname = sorted(glob.glob("/tmp/ACO*csv"))
        if len(fname) > 0:
            fname = fname[-1]
            print(f"{fname} : size {os.path.getsize(fname)}")
            print()
        time.sleep(sleep_time)


def test_logger_buffer_normal(ss: acsense_udp.UdpSocketIn):
    test_logger_buffer(ss, 0.2)


def test_logger_buffer_slow(ss: acsense_udp.UdpSocketIn):
    test_logger_buffer(ss, 1)


def test_logger_no_buffer(ss: acsense_udp.UdpSocketIn):
    hh = acsense_udp.InterfaceHelper()
    hh.add_socket(ss.use_mcast, ss.iface_ip, ss.port, ss.mcast_group)
    hh.enable_aco_fork(True)
    hh.enable_logger(acsense_udp.LOGGER.ACO_CSV, True)
    hh.enable_buffer_aco(False)

    hh.run_threads()

    while True:
        print("Test tick -- main thread")
        fname = sorted(glob.glob("/tmp/ACO*csv"))
        if len(fname) > 0:
            fname = fname[-1]
            print(f"{fname} : size {os.path.getsize(fname)}")
            print()
        time.sleep(1)


if __name__ == "__main__":
    acsense_udp.init_logger()

    acsense_udp.set_verbose(5)
    ss1 = acsense_udp.UdpSocketIn()
    print(ss1)

    ss1 = acsense_udp.UdpSocketIn(True, "192.168.1.115", 9760, "224.1.1.1")
    print(ss1)

    for key, val in {
        "Test socket : single-thread collection": test_single_thread,
        "Test socket : multithread collection": test_multithread,
        "Test interface helper : multithread collection + logging (read buffer normally)": test_logger_buffer_normal,
        "Test interface helper : multithread collection + logging (read buffer infrequently)": test_logger_buffer_slow,
        "Test interface helper : multithread collection + logging (skip buffer step)": test_logger_no_buffer,
    }.items():
        print("=" * 50 + "\n\n")
        print(key)
        p = multiprocessing.Process(target=val, name=key, args=(ss1,))
        p.start()

        # Wait 10 seconds for foo
        time.sleep(10)

        # Terminate foo
        p.terminate()

        # Cleanup
        p.join()

        print(key + " completed\n")
