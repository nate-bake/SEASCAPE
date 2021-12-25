import json
import os
import sys
import datetime
from dateutil.relativedelta import relativedelta


def ask_proceed():
    print()
    while "the answer is invalid":
        reply = (
            str(input("Would you like to continue the launch? [Y/N]: ")).upper().strip()
        )
        if reply == "Y":
            print("Ok. Continuing...\n")
            return True
        if reply == "N":
            return False


def check_key_structure(obj, structure, path):
    try:
        for key, value in structure.items():
            path += "/" + key
            o = obj[key]
            error, path = check_key_structure(o, value, path)
            if error:
                return error, path
        return "", path.rsplit("/", 1)[0]
    except KeyError:
        return key, path.rsplit("/", 1)[0]


def check_calibration_date(filepath):
    mtime = os.path.getmtime(filepath)
    modified = datetime.datetime.fromtimestamp(mtime)
    a_month_ago = datetime.datetime.now() - relativedelta(months=1)
    if modified < a_month_ago:
        return modified
    return None


def check_all():

    cfg = json.load(open("config.json"))

    ########################## MAKE SURE ATTRIBUTES ARE PRESENT ############################

    path = ""
    structure = {
        "THREADS": {
            "LOGGER": {
                "ENABLED": {},
                "RATE": {},
                "LOG_SENSOR_DATA": {},
                "LOG_ESTIMATOR_0": {},
                "LOG_ESTIMATOR_1": {},
                "LOG_CONTROLLER_0": {},
                "LOG_CONTROLLER_1": {},
                "LOG_RCIN_SERVO": {},
            },
            "ESTIMATOR_0": {"ENABLED": {}, "RATE": {}},
            "ESTIMATOR_1": {"ENABLED": {}, "RATE": {}},
            "CONTROLLER_0": {
                "ENABLED": {},
                "RATE": {},
                "XH_VECTOR_TO_USE": {},
            },
            "CONTROLLER_1": {
                "ENABLED": {},
                "RATE": {},
                "XH_VECTOR_TO_USE": {},
            },
            "IMU_ADC": {
                "ENABLED": {},
                "RATE": {},
                "USE_LSM9DS1": {},
                "USE_MPU9250": {},
                "PRIMARY_IMU": {},
                "APPLY_CALIBRATION_PROFILE": {},
                "USE_ADC": {},
            },
            "GPS_BAROMETER": {
                "ENABLED": {},
                "RATE": {},
                "USE_GPS": {},
                "USE_MS5611": {},
            },
            "RCIN_SERVO": {
                "ENABLED": {},
                "RATE": {},
                "PWM_FREQUENCY": {},
                "CONTROLLER_VECTOR_TO_USE": {},
                "ELEVATOR_CHANNEL": {},
                "AILERON_CHANNEL": {},
                "FLIGHT_MODES": {
                    "MODE_CHANNEL": {},
                    "MANUAL_RANGE": {"LOW": {}, "HIGH": {}},
                    "SEMI-AUTO_RANGE": {"LOW": {}, "HIGH": {}},
                    "SEMI-AUTO_DEADZONE": {},
                    "AUTO_RANGE": {"LOW": {}, "HIGH": {}},
                },
                "MIN_PWM_OUT": {},
                "MAX_PWM_OUT": {},
            },
            "TELEMETRY": {"ENABLED": {}, "RATE": {}},
        }
    }

    key, path = check_key_structure(cfg, structure, path)

    if key:
        print(
            f"CONFIG ERROR: Attribute {key} is missing in path '{path}' of config.json.\n"
        )
        return None, None

    ################################ VERIFY THREAD RATES ##################################

    threads = cfg["THREADS"]

    for name in [
        "LOGGER",
        "ESTIMATOR_0",
        "ESTIMATOR_1",
        "CONTROLLER_0",
        "CONTROLLER_1",
        "IMU_ADC",
        "GPS_BAROMETER",
        "RCIN_SERVO",
        "TELEMETRY",
    ]:
        t = threads[name]
        try:
            if t["RATE"] <= 0.0 and t["ENABLED"]:
                print(
                    f"CONFIG ERROR: Invalid rate specified for {name} thread. Rate must be greater than 0 hertz.\nIf you wish to disable the thread, set ENABLED to false.\n"
                )
                return None, None
        except TypeError:
            print(
                f"CONFIG ERROR:\n\tInvalid type for ENABLED and/or RATE attributes of {name} thread.\nENABLED should be boolean, RATE should be numeric.\n"
            )
            return None, None

    ############################# CHECK CONTROLLER-XH DEPENDENCIES ############################

    for name in [
        "CONTROLLER_0",
        "CONTROLLER_1",
    ]:
        t = threads[name]
        if not t["ENABLED"]:
            continue
        xh = t["XH_VECTOR_TO_USE"]
        if xh not in [0, 1]:
            print(
                f"CONFIG ERROR: Invalid value for THREADS/{name}/XH_VECTOR_TO_USE. Should be either 0 or 1.\n"
            )
        if not threads["ESTIMATOR_" + str(xh)]["ENABLED"]:
            print(
                f"CONFIG WARNING: {name} is expecting to read values from ESTIMATOR_{xh} which is disabled.\nHaving the xh_{xh} vector empty will likely yield dangerously innacurate results from {name}."
            )
            if not ask_proceed():
                return None, None

    ########################## CHECK SERVO-CONTROLLER DEPENDENCY ###########################

    t = threads["RCIN_SERVO"]
    if t["ENABLED"]:
        i = t["CONTROLLER_VECTOR_TO_USE"]
        if i not in [0, 1]:
            print(
                f"CONFIG ERROR: Invalid value for THREADS/RCIN_SERVO/CONTROLER_VECTOR_TO_USE. Should be either 0 or 1.\n"
            )
        if not threads["CONTROLLER_" + str(i)]["ENABLED"]:
            print(
                f"CONFIG WARNING: RCIN_SERVO is expecting to read values from CONTROLLER_{i} which is disabled.\nHaving the CONTROLLER_{i} vector empty will likely yield dangerously innacurate servo outputs."
            )
            if not ask_proceed():
                return None, None

    ############################### CHECK IMU CALIBRATION FILES ##############################

    imu_cfg = cfg["THREADS"]["IMU_ADC"]
    if imu_cfg["ENABLED"] and imu_cfg["APPLY_CALIBRATION_PROFILE"]:
        if imu_cfg["USE_LSM9DS1"]:
            if not os.path.exists("data/calibration/LSM9DS1_calibration.bin"):
                print(
                    "Looks like you haven't calibrated your LSM9DS1 IMU yet. Try launching again after running the calibration script.\n"
                )
                sys.exit()
            modified = check_calibration_date(
                "data/calibration/LSM9DS1_calibration.bin"
            )
            if modified is not None:
                print(
                    f"Looks like you haven't calibrated your LSM9DS1 IMU in over a month. [{modified.strftime('%m/%d/%Y, %H:%M:%S')}]"
                )
                if not ask_proceed():
                    sys.exit()
        if imu_cfg["USE_MPU9250"]:
            if not os.path.exists("data/calibration/MPU9250_calibration.bin"):
                print(
                    "Looks like you haven't calibrated your MPU9250 IMU yet. Try launching again after running the calibration script.\n"
                )
                sys.exit()
            modified = check_calibration_date(
                "data/calibration/MPU9250_calibration.bin"
            )
            if modified is not None:
                print(
                    f"Looks like you haven't calibrated your MPU9250 IMU in over a month. [{modified.strftime('%m/%d/%Y, %H:%M:%S')}]"
                )
                if not ask_proceed():
                    sys.exit()

    ########################### LOAD KEYS AND CHECK FOR DUPLICATES ############################

    i = 1
    vectors = cfg["VECTORS"]

    keys = {}

    for v in vectors:
        ks = v["KEYS"]
        for k in ks:
            k = v["ID"] + "_" + k
            if k in keys.keys():
                print(f"CONFIG ERROR: Duplicate keys '{k}' found in config.json.\n")
                return None, None
            keys[k] = i
            i += 1

    return cfg, keys


if __name__ == "__main__":
    check_all()
