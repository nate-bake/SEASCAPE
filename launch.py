import os, sys
import time
import subprocess
import multiprocessing
import sysv_ipc as ipc
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
    return {key: keys[key] for key in keys.keys() if key.startswith(tuple(vec_list))}


if __name__ == "__main__":

    print()
    if not os.path.exists("core/air"):
        print(
            "Looks like the core files have not been compiled yet.\nAttempting to compile...\n"
        )
        os.system("make -s -C core/")
        if not os.path.exists("core/air"):
            print("Build failed. Launch canceled.")
            sys.exit()
        else:
            print("Compilation successful.\n")
            time.sleep(1)

    cfg, keys = prelaunch_checks.check_all()
    if keys is None:
        sys.exit()

    process = subprocess.Popen(["sudo", "./core/air"])
    print(f"Core threads have launched.\t\t[PID:{process.pid}]")

    time.sleep(3)

    ftok_path = "core/air.h"
    key = ipc.ftok(
        ftok_path, ord("~"), silence_warning=True
    )  # refer to shared memory created by core/air
    shm = ipc.SharedMemory(key, 0, 0)

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
        mem_con1 = shared_mem_helper_controller_1.helper(shm, keys, con1_xh_index)
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
