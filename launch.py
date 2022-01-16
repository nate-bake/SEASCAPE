import sys
import time
import subprocess
import multiprocessing
from core import prelaunch_checks
from core import shared_mem_helper_estimator_1
from core import shared_mem_helper_controller_1
import estimator_1
import controller_1
from core import logger


def get_log_keys():
    vec_list = []
    if cfg["THREADS"]["LOGGER"]["LOG_SENSOR_DATA"]:
        vec_list.append("y_")
    if cfg["THREADS"]["LOGGER"]["LOG_ESTIMATOR_0"]:
        vec_list.append("xh_0_")
    if cfg["THREADS"]["LOGGER"]["LOG_ESTIMATOR_1"]:
        vec_list.append("xh_1_")
    if cfg["THREADS"]["LOGGER"]["LOG_CONTROLLER_0"]:
        vec_list.append("controller_0_")
    if cfg["THREADS"]["LOGGER"]["LOG_CONTROLLER_1"]:
        vec_list.append("controller_1_")
    if cfg["THREADS"]["LOGGER"]["LOG_RCIN_SERVO"]:
        vec_list.append("rcin_")
        vec_list.append("servo_")
    log_keys = {
        key: keys[key] for key in keys.keys() if key.startswith(tuple(vec_list))
    }
    for k in log_keys.keys():
        if k.endswith(
            "CHANNEL_" + str(cfg["THREADS"]["RCIN_SERVO"]["THROTTLE_CHANNEL"])
        ):
            log_keys[k.split("CHANNEL")[0] + " [THROTTLE]"] = log_keys.pop(k)
        if k.endswith(
            "CHANNEL_" + str(cfg["THREADS"]["RCIN_SERVO"]["ELEVATOR_CHANNEL"])
        ):
            log_keys[k.split("CHANNEL")[0] + " [ELEVATOR]"] = log_keys.pop(k)
        if k.endswith(
            "CHANNEL_" + str(cfg["THREADS"]["RCIN_SERVO"]["AILERON_CHANNEL"])
        ):
            log_keys[k.split("CHANNEL")[0] + " [AILERON]"] = log_keys.pop(k)
        if k.endswith("CHANNEL_" + str(cfg["THREADS"]["RCIN_SERVO"]["RUDDER_CHANNEL"])):
            log_keys[k.split("CHANNEL")[0] + " [RUDDER]"] = log_keys.pop(k)
        if k.endswith("CHANNEL_" + str(cfg["THREADS"]["RCIN_SERVO"]["FLAPS_CHANNEL"])):
            log_keys[k.split("CHANNEL")[0] + " [FLAPS]"] = log_keys.pop(k)
    return log_keys


def get_servo_channels():
    channels = {}
    channels["THROTTLE"] = cfg["THREADS"]["RCIN_SERVO"]["THROTTLE_CHANNEL"]
    channels["ELEVATOR"] = cfg["THREADS"]["RCIN_SERVO"]["ELEVATOR_CHANNEL"]
    channels["AILERON"] = cfg["THREADS"]["RCIN_SERVO"]["AILERON_CHANNEL"]
    channels["RUDDER"] = cfg["THREADS"]["RCIN_SERVO"]["RUDDER_CHANNEL"]
    channels["FLAPS"] = cfg["THREADS"]["RCIN_SERVO"]["FLAPS_CHANNEL"]
    channels["MODE"] = cfg["THREADS"]["RCIN_SERVO"]["FLIGHT_MODES"]["MODE_CHANNEL"]
    return channels


if __name__ == "__main__":

    print()

    prelaunch_checks.check_dependencies()
    time.sleep(1)
    prelaunch_checks.check_core()
    time.sleep(1)
    cfg, keys = prelaunch_checks.check_config()

    process = subprocess.Popen(["sudo", "./core/air"])
    print(f"Core threads have launched.\t\t[PID:{process.pid}]")

    time.sleep(3)

    import sysv_ipc as ipc

    ftok_path = "core/air.h"
    key = ipc.ftok(ftok_path, ord("~"), silence_warning=True)
    shm = ipc.SharedMemory(key, 0, 0)
    # attach to shared memory created by core/air
    shm.attach(0, 0)

    if cfg["THREADS"]["LOGGER"]["ENABLED"]:
        max_sleep_logger = 1.0 / cfg["THREADS"]["LOGGER"]["RATE"]
        log_keys = get_log_keys()
        logger_process = multiprocessing.Process(
            target=logger.logger_loop,
            args=(shm, log_keys, max_sleep_logger),
        )
        logger_process.daemon = True
        logger_process.start()

    if cfg["THREADS"]["ESTIMATOR_1"]["ENABLED"]:
        mem_est1 = shared_mem_helper_estimator_1.helper(shm, keys)
        max_sleep_est1 = 1.0 / cfg["THREADS"]["ESTIMATOR_1"]["RATE"]
        estimator_1_process = multiprocessing.Process(
            target=estimator_1.estimator_loop, args=(mem_est1, max_sleep_est1)
        )
        estimator_1_process.daemon = True
        estimator_1_process.start()

    if cfg["THREADS"]["CONTROLLER_1"]["ENABLED"]:

        con1_xh_index = cfg["THREADS"]["CONTROLLER_1"]["XH_VECTOR_TO_USE"]
        channels = get_servo_channels()
        mem_con1 = shared_mem_helper_controller_1.helper(
            shm, keys, con1_xh_index, channels
        )
        max_sleep_con1 = 1.0 / cfg["THREADS"]["CONTROLLER_1"]["RATE"]
        controller_1_process = multiprocessing.Process(
            target=controller_1.controller_loop, args=(mem_con1, max_sleep_con1)
        )
        controller_1_process.daemon = True
        controller_1_process.start()

    shm.remove()  # when all processes detach, the memory will be cleared.

    process.wait()
    print("\nCore process has died.\nShutting down...")
    sys.exit()
