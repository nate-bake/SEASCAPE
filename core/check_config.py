import json


def ask_proceed():
    print()
    while "the answer is invalid":
        reply = (
            str(input("Would you like to continue the launch? [Y/N]: ")).upper().strip()
        )
        if reply == "Y":
            print('Ok. Continuing...\n')
            return True
        if reply == "N":
            return False


def check(obj, structure, path):
    try:
        for key, value in structure.items():
            path += "/" + key
            o = obj[key]
            error, path = check(o, value, path)
            if error:
                return error, path
        return "", path.rsplit("/", 1)[0]
    except KeyError:
        return key, path.rsplit("/", 1)[0]


def check_config(filepath):

    cfg = json.load(open(filepath))

    ########################## MAKE SURE ATTRIBUTES ARE PRESENT ############################

    path = ""
    structure = {
        "THREADS": {
            "ESTIMATOR_0": {"ENABLED": {}, "RATE": {}, "LOG_XH_DATA": {}},
            "ESTIMATOR_1": {"ENABLED": {}, "RATE": {}, "LOG_XH_DATA": {}},
            "CONTROLLER_0": {
                "ENABLED": {},
                "RATE": {},
                "XH_VECTOR_TO_USE": {},
                "LOG_CONTROLLER_DATA": {},
            },
            "CONTROLLER_1": {
                "ENABLED": {},
                "RATE": {},
                "XH_VECTOR_TO_USE": {},
                "LOG_CONTROLLER_DATA": {},
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

    key, path = check(cfg, structure, path)

    if key:
        print(
            f"CONFIG ERROR:\n\tAttribute {key} is missing in path '{path}' of your config file."
        )
        return None, None

    ################################ VERIFY THREAD RATES ##################################

    threads = cfg["THREADS"]

    for name in [
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
                    f"CONFIG ERROR:\n\tInvalid rate specified for {name} thread. Rate must be greater than 0 hertz.\n\tIf you wish to disable the thread, set ENABLED to false."
                )
                return None, None
        except TypeError:
            print(
                f"CONFIG ERROR:\n\tInvalid type for ENABLED and/or RATE attributes of {name} thread.\n\tENABLED should be boolean, RATE should be numeric."
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
                f"CONFIG ERROR:\n\tInvalid value for THREADS/{name}/XH_VECTOR_TO_USE. Should be either 0 or 1."
            )
        if not threads["ESTIMATOR_" + str(xh)]["ENABLED"]:
            print(
                f"CONFIG WARNING:\n\t{name} is expecting to read values from ESTIMATOR_{xh} which is disabled.\n\tHaving the xh_{xh} vector empty will likely yield dangerously innacurate results from {name}."
            )
            if not ask_proceed():
                return None, None

    ########################## CHECK SERVO-CONTROLLER DEPENDENCY ###########################

    t = threads["RCIN_SERVO"]
    if t["ENABLED"]:
        i = t["CONTROLLER_VECTOR_TO_USE"]
        if i not in [0, 1]:
            print(
                f"CONFIG ERROR:\n\tInvalid value for THREADS/RCIN_SERVO/CONTROLER_VECTOR_TO_USE. Should be either 0 or 1."
            )
        if not threads["CONTROLLER_" + str(i)]["ENABLED"]:
            print(
                f"CONFIG WARNING:\n\tRCIN_SERVO is expecting to read values from CONTROLLER_{i} which is disabled.\n\tHaving the CONTROLLER_{i} vector empty will likely yield dangerously innacurate servo outputs."
            )
            if not ask_proceed():
                return None, None

    i = 1
    vectors = cfg["VECTORS"]

    keys = {}

    for v in vectors:
        ks = v["KEYS"]
        for k in ks:
            k = v["ID"] + "_" + k
            if k in keys.keys():
                print(f"DUPLICATE KEYS '{k}' FOUND IN CONFIG FILE.")
                return None, None
            keys[k] = i
            i += 1

    return cfg, keys


if __name__ == "__main__":
    check_config()
