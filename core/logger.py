import time
import datetime
import struct
import numpy as np
import atexit
import signal


def logger_loop(shm, log_keys, max_sleep):

    data = []
    header = "TIMESTAMP, "
    for k in log_keys.keys():
        header += k + ", "
    start_time = time.time()

    def save_log_file(*args):
        csv = np.array(data)
        now = datetime.datetime.fromtimestamp(int(start_time))
        filename = "data/logs/" + now.strftime("%Y-%m-%d__%H.%M.%S") + ".csv"
        np.savetxt(filename, csv, fmt="%.4f", delimiter=", ", header=header)

    atexit.register(save_log_file)
    signal.signal(signal.SIGINT, save_log_file)

    while True:
        initial_time = time.time()
        row = [
            struct.unpack("d", shm.read(8, index * 8))[0] for index in log_keys.values()
        ]
        data.append(np.array([time.time() - start_time] + row))
        time.sleep(max_sleep - (time.time() - initial_time))
