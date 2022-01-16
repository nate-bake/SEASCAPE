import time
import datetime
import struct
import numpy as np
import atexit
import signal


def get_log_keys(cfg, keys, channels):
    vec_list = []
    if cfg["THREADS"]["LOGGER"]["LOG_SENSOR_DATA"]:
        vec_list.append("y_")
    if cfg["THREADS"]["LOGGER"]["LOG_ESTIMATOR_0"] and cfg["THREADS"]["ESTIMATOR_0"]["ENABLED"]:
        vec_list.append("xh_0_")
    if cfg["THREADS"]["LOGGER"]["LOG_ESTIMATOR_1"] and cfg["THREADS"]["ESTIMATOR_1"]["ENABLED"]:
        vec_list.append("xh_1_")
    if cfg["THREADS"]["LOGGER"]["LOG_CONTROLLER_0"] and cfg["THREADS"]["CONTROLLER_0"]["ENABLED"]:
        vec_list.append("controller_0_")
    if cfg["THREADS"]["LOGGER"]["LOG_CONTROLLER_1"] and cfg["THREADS"]["CONTROLLER_1"]["ENABLED"]:
        vec_list.append("controller_1_")
    if cfg["THREADS"]["LOGGER"]["LOG_RCIN_SERVO"] and cfg["THREADS"]["RCIN_SERVO"]["ENABLED"]:
        vec_list.append("rcin_")
        vec_list.append("servo_")
    log_keys = {}
    for key in keys.keys():
        if key.startswith(tuple(vec_list)):
            if "ADC" in key or "BARO_INIT" in key:
                continue  # skip these columns to save storage.
            if "IMU_2" in key and (
                (not cfg["THREADS"]["IMU_ADC"]["USE_LSM9DS1"]) or (not cfg["THREADS"]["IMU_ADC"]["USE_MPU9250"])
            ):
                continue
            if "GPS" in key and (not cfg["THREADS"]["GPS_BAROMETER"]["USE_GPS"]):
                continue
            if "BARO" in key and (not cfg["THREADS"]["GPS_BAROMETER"]["USE_MS5611"]):
                continue
            log_keys[key] = keys[key]
            # rename channel columns to reflect servo type
            if key.endswith("CHANNEL_" + str(channels["THROTTLE"])):
                log_keys[key.split("CHANNEL")[0] + "THROTTLE"] = log_keys.pop(key)
            if key.endswith("CHANNEL_" + str(channels["ELEVATOR"])):
                log_keys[key.split("CHANNEL")[0] + "ELEVATOR"] = log_keys.pop(key)
            if key.endswith("CHANNEL_" + str(channels["AILERON"])):
                log_keys[key.split("CHANNEL")[0] + "AILERON"] = log_keys.pop(key)
            if key.endswith("CHANNEL_" + str(channels["RUDDER"])):
                log_keys[key.split("CHANNEL")[0] + "RUDDER"] = log_keys.pop(key)
            if key.endswith("CHANNEL_" + str(channels["FLAPS"])):
                log_keys[key.split("CHANNEL")[0] + "FLAPS"] = log_keys.pop(key)
            if key.endswith("rcin_CHANNEL_" + str(channels["MODE"])):
                log_keys[key.split("CHANNEL")[0] + "MODE"] = log_keys.pop(key)
    return log_keys


def logger_loop(shm, log_keys, interval, max_sleep):

    data = []
    header = "TIMESTAMP, "
    for k in log_keys.keys():
        header += k + ", "
    start_time = time.time()

    now = datetime.datetime.fromtimestamp(int(start_time))
    filename = "data/logs/" + now.strftime("%Y-%m-%d_%H.%M.%S") + ".csv"

    with open(filename, "ab") as f:
        np.savetxt(f, [], fmt="%s", delimiter=", ", header=header)

        def save_log_file(*args):
            csv = np.array(data, dtype="str")
            np.savetxt(f, csv, fmt="%s", delimiter=", ")

        atexit.register(save_log_file)
        signal.signal(signal.SIGINT, save_log_file)

        mode_flag_index = None
        if "servo_MODE_FLAG" in log_keys:
            mode_flag_index = list(log_keys.keys()).index("servo_MODE_FLAG")

        i = 0
        while True:
            initial_time = time.time()
            row = [struct.unpack("d", shm.read(8, index * 8))[0] for index in log_keys.values()]
            if mode_flag_index:
                if row[mode_flag_index] == 0:
                    row[mode_flag_index] = "MANUAL"
                elif row[mode_flag_index] == 1:
                    row[mode_flag_index] = "SEMI-AUTO"
                elif row[mode_flag_index] == 2:
                    row[mode_flag_index] = "AUTO"
            stamp = time.time() - start_time
            data.append(np.array([stamp] + row))
            if interval > 0 and stamp >= interval * i:
                save_log_file()
                data = []
                i += 1
            else:
                sleep_time = max_sleep - (time.time() - initial_time)
                time.sleep(max(sleep_time, 0))
