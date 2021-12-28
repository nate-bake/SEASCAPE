#include "air_config.h"

air_config::air_config() {

    Json::Reader reader;
    Json::Value cfg;

    std::ifstream file("config.json");

    if (!reader.parse(file, cfg)) {
        std::cout << reader.getFormattedErrorMessages();
        exit(1);
    }

    // we can assume config file is properly setup at this point.
    // thanks to prelaunch_checks.py

    auto threads = cfg["THREADS"];

    IMU_LOOP_RATE = threads["IMU_ADC"]["RATE"].asDouble();
    GPS_LOOP_RATE = threads["GPS_BAROMETER"]["RATE"].asDouble();
    SERVO_LOOP_RATE = threads["RCIN_SERVO"]["RATE"].asDouble();
    ESTIMATION_LOOP_RATE = threads["ESTIMATOR_0"]["RATE"].asDouble();
    CONTROL_LOOP_RATE = threads["CONTROLLER_0"]["RATE"].asDouble();
    TELEMETRY_LOOP_RATE = threads["TELEMETRY"]["RATE"].asDouble();

    ESTIMATOR_ENABLED = threads["ESTIMATOR_0"]["ENABLED"].asBool();
    CONTROLLER_ENABLED = threads["CONTROLLER_0"]["ENABLED"].asBool();
    CONTROLLER_XH = threads["CONTROLLER_0"]["XH_VECTOR_TO_USE"].asInt();
    SERVO_CONTROLLER = threads["RCIN_SERVO"]["CONTROLLER_VECTOR_TO_USE"].asInt();
    SERVO_LOOP_ENABLED = threads["RCIN_SERVO"]["ENABLED"].asBool();
    TELEMETRY_LOOP_ENABLED = threads["TELEMETRY"]["ENABLED"].asBool();
    GPS_LOOP_ENABLED = threads["GPS_BAROMETER"]["ENABLED"].asBool();
    IMU_LOOP_ENABLED = threads["IMU_ADC"]["ENABLED"].asBool();
    LSM_ENABLED = threads["IMU_ADC"]["USE_LSM9DS1"].asBool();
    MPU_ENABLED = threads["IMU_ADC"]["USE_MPU9250"].asBool();
    USE_IMU_CALIBRATION = threads["IMU_ADC"]["APPLY_CALIBRATION_PROFILE"].asBool();
    ADC_ENABLED = threads["IMU_ADC"]["USE_ADC"].asBool();
    GPS_ENABLED = threads["GPS_BAROMETER"]["USE_GPS"].asBool();
    MS5611_ENABLED = threads["GPS_BAROMETER"]["USE_MS5611"].asBool();
    PRIMARY_IMU = threads["IMU_ADC"]["PRIMARY_IMU"].asString();
    if (PRIMARY_IMU != "LSM9DS1" && PRIMARY_IMU != "MPU9250") {
        if (!LSM_ENABLED && MPU_ENABLED) {
            PRIMARY_IMU = "MPU9250";
        } else {
            PRIMARY_IMU = "LSM9DS1";
        }
    }

    PWM_FREQUENCY = threads["RCIN_SERVO"]["PWM_FREQUENCY"].asInt();
    MIN_PWM_OUT = threads["RCIN_SERVO"]["MIN_PWM_OUT"].asInt();
    MAX_PWM_OUT = threads["RCIN_SERVO"]["MAX_PWM_OUT"].asInt();
    ELEVATOR_CHANNEL = threads["RCIN_SERVO"]["ELEVATOR_CHANNEL"].asInt();
    AILERON_CHANNEL = threads["RCIN_SERVO"]["AILERON_CHANNEL"].asInt();
    FLIGHT_MODE_CHANNEL = threads["RCIN_SERVO"]["FLIGHT_MODES"]["MODE_CHANNEL"].asInt();
    MANUAL_MODE_MIN = threads["RCIN_SERVO"]["FLIGHT_MODES"]["MANUAL_RANGE"]["LOW"].asInt();
    MANUAL_MODE_MAX = threads["RCIN_SERVO"]["FLIGHT_MODES"]["MANUAL_RANGE"]["HIGH"].asInt();
    SEMI_MODE_MIN = threads["RCIN_SERVO"]["FLIGHT_MODES"]["SEMI-AUTO_RANGE"]["LOW"].asInt();
    SEMI_MODE_MAX = threads["RCIN_SERVO"]["FLIGHT_MODES"]["SEMI-AUTO_RANGE"]["HIGH"].asInt();
    SEMI_DEADZONE = threads["RCIN_SERVO"]["FLIGHT_MODES"]["SEMI-AUTO_DEADZONE"].asInt();
    AUTO_MODE_MIN = threads["RCIN_SERVO"]["FLIGHT_MODES"]["AUTO_RANGE"]["LOW"].asInt();
    AUTO_MODE_MAX = threads["RCIN_SERVO"]["FLIGHT_MODES"]["AUTO_RANGE"]["HIGH"].asInt();

    int i = 1; // leave index 0 empty for default when key not found
    auto vectors = cfg["VECTORS"];

    for (int v = 0; v < vectors.size(); v++) {
        auto vector = vectors[v];
        auto ks = vector["KEYS"];
        for (int k = 0; k < ks.size(); k++) {
            std::string key = vector["ID"].asString() + "_" + ks[k].asString();
            keys.insert(std::pair<std::string, int>(key, i++));
        }
    }
}