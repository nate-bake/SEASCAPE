import os, sys
import struct
import time
import datetime
from dateutil.relativedelta import relativedelta
import subprocess
import multiprocessing
import sysv_ipc as ipc
from core import check_config
from core import shared_mem_helper_estimator_1
from core import shared_mem_helper_controller_1
import estimator_1
import controller_1


def check_calibration_date(filepath):
    mtime = os.path.getmtime(filepath)
    modified = datetime.datetime.fromtimestamp(mtime)
    a_month_ago = datetime.datetime.now() - relativedelta(months=1)
    if modified < a_month_ago:
        return modified
    return None


if __name__ == "__main__":

    config_path = "config.json"

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

    cfg, keys = check_config.check_config(config_path)
    if keys is None:
        sys.exit()

    if cfg["THREADS"]["IMU_ADC"]["ENABLED"]:
        if cfg["THREADS"]["IMU_ADC"]["USE_LSM9DS1"]:
            if not os.path.exists("data/calibration/LSM9DS1_calibration.bin"):
                print(
                    "Looks like you haven't calibrated your LSM9DS1 IMU yet. Try launching again after running the calibration script."
                )
                sys.exit()
            modified = check_calibration_date(
                "data/calibration/LSM9DS1_calibration.bin"
            )
            if modified is not None:
                print(
                    f"Looks like you haven't calibrated your LSM9DS1 IMU in over a month. [{modified.strftime('%m/%d/%Y, %H:%M:%S')}]"
                )
                if not check_config.ask_proceed():
                    sys.exit()
        if cfg["THREADS"]["IMU_ADC"]["USE_MPU9250"]:
            if not os.path.exists("data/calibration/MPU9250_calibration.bin"):
                print(
                    "Looks like you haven't calibrated your MPU9250 IMU yet. Try launching again after running the calibration script."
                )
                sys.exit()
            modified = check_calibration_date(
                "data/calibration/MPU9250_calibration.bin"
            )
            if modified is not None:
                print(
                    f"Looks like you haven't calibrated your MPU9250 IMU in over a month. [{modified.strftime('%m/%d/%Y, %H:%M:%S')}]"
                )
                if not check_config.ask_proceed():
                    sys.exit()

    process = subprocess.Popen(["sudo", "./core/air", config_path])
    print(f"Core threads have launched.\t\t[PID:{process.pid}]")

    time.sleep(5)

    ftok_path = "core/air.h"
    key = ipc.ftok(
        ftok_path, ord("~"), silence_warning=True
    )  # refer to shared memory created by core/air
    shm = ipc.SharedMemory(key, 0, 0)

    shm.attach(0, 0)

    con1_xh_index = cfg["THREADS"]["CONTROLLER_1"]["XH_VECTOR_TO_USE"]

    mem_est1 = shared_mem_helper_estimator_1.helper(shm, keys)
    mem_con1 = shared_mem_helper_controller_1.helper(shm, keys, con1_xh_index)

    max_sleep_est1 = 1.0 / cfg["THREADS"]["ESTIMATOR_1"]["RATE"]
    max_sleep_con1 = 1.0 / cfg["THREADS"]["CONTROLLER_1"]["RATE"]

    estimator_1_process = multiprocessing.Process(
        target=estimator_1.estimator_loop, args=(mem_est1, max_sleep_est1)
    )
    estimator_1_process.daemon = True
    estimator_1_process.start()

    controller_1_process = multiprocessing.Process(
        target=controller_1.controller_loop, args=(mem_con1, max_sleep_con1)
    )
    controller_1_process.daemon = True
    controller_1_process.start()

    shm.remove()  # when all processes detach, the memory will be cleared.

    process.wait()
    print("\nCore process has died.\nShutting down...")
    sys.exit()

    # estimator_1_process.join()
    # controller_1_process.join()
