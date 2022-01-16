#pragma once

#include <map>
#include <string>

class air_config {

public:
    std::map<std::string, int> keys;
    double IMU_LOOP_RATE;
    double GPS_LOOP_RATE;
    double SERVO_LOOP_RATE;
    double ESTIMATION_LOOP_RATE;
    double CONTROL_LOOP_RATE;
    double TELEMETRY_LOOP_RATE;

    bool ESTIMATOR_ENABLED;
    bool CONTROLLER_ENABLED;
    int CONTROLLER_XH;
    int SERVO_CONTROLLER;
    bool SERVO_LOOP_ENABLED;
    bool TELEMETRY_LOOP_ENABLED;
    bool GPS_LOOP_ENABLED;
    bool IMU_LOOP_ENABLED;
    bool LSM_ENABLED;
    bool MPU_ENABLED;
    bool ADC_ENABLED;
    bool MS5611_ENABLED;
    bool GPS_ENABLED;
    std::string PRIMARY_IMU;
    bool USE_IMU_CALIBRATION;

    float PWM_FREQUENCY;
    int MIN_THROTTLE;
    int MAX_THROTTLE;
    int MIN_SERVO;
    int MAX_SERVO;
    int THROTTLE_CHANNEL;
    int AILERON_CHANNEL;
    int ELEVATOR_CHANNEL;
    int RUDDER_CHANNEL;
    int FLAPS_CHANNEL;
    int FLIGHT_MODE_CHANNEL;
    int MANUAL_MODE_MIN;
    int MANUAL_MODE_MAX;
    int SEMI_MODE_MIN;
    int SEMI_MODE_MAX;
    int SEMI_DEADZONE;
    int AUTO_MODE_MIN;
    int AUTO_MODE_MAX;

    air_config();

};