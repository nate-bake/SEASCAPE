#include "air.h"

void* estimation_loop(void* arguments) {
    usleep(5000000);
    thread_struct* args = (thread_struct*)arguments;
    double* array = args->array;
    const air_config* cfg = args->cfg;
    std::map<std::string, int> keys = cfg->keys;
    int imu = 0, gps = 0;
    struct imu_calibration_profile calibration_profile;
    if (!cfg->USE_IMU_CALIBRATION) { // apply calibration in estimator rather than at sensor read.
        if (cfg->PRIMARY_IMU == "LSM9DS1") {
            load_calibration_file("LSM9DS1", &calibration_profile);
        } else if (cfg->PRIMARY_IMU == "MPU9250") {
            load_calibration_file("MPU9250", &calibration_profile);
        }
    }
    int max_sleep = hertz_to_microseconds(cfg->ESTIMATION_LOOP_RATE);
    uint64_t start_time, now;

    while (true) {
        start_time = current_time_microseconds();
        if (imu == array[keys["y_IMU_UPDATES"]]) {
            continue;
        }
        imu = array[keys["y_IMU_UPDATES"]];
        double ax = array[keys["y_IMU_1_AX"]];
        double ay = array[keys["y_IMU_1_AY"]];
        double az = array[keys["y_IMU_1_AZ"]];
        double gx = array[keys["y_IMU_1_GYRO_P"]];
        double gy = array[keys["y_IMU_1_GYRO_Q"]];
        double gz = array[keys["y_IMU_1_GYRO_R"]];
        double mx = array[keys["y_IMU_1_MAG_X"]];
        double my = array[keys["y_IMU_1_MAG_Y"]];
        double mz = array[keys["y_IMU_1_MAG_Z"]];

        if (!cfg->USE_IMU_CALIBRATION && calibration_profile.offsets != nullptr) {
            // must apply calibration here rather than at sensor read.
            ax -= calibration_profile.offsets[0];
            ay -= calibration_profile.offsets[1];
            az -= calibration_profile.offsets[2];
            gx -= calibration_profile.offsets[3];
            gy -= calibration_profile.offsets[4];
            gz -= calibration_profile.offsets[5];
            mx -= calibration_profile.offsets[6];
            my -= calibration_profile.offsets[7];
            mz -= calibration_profile.offsets[8];

            mx = mx * calibration_profile.matrix[0] + my * calibration_profile.matrix[3] + mz * calibration_profile.matrix[6];
            my = mx * calibration_profile.matrix[1] + my * calibration_profile.matrix[4] + mz * calibration_profile.matrix[7];
            mz = mx * calibration_profile.matrix[2] + my * calibration_profile.matrix[5] + mz * calibration_profile.matrix[8];
        }

        if (gps != array[keys["y_GPS_UPDATES"]]) {
            gps = array[keys["y_GPS_UPDATES"]];
            // ESTIMATION WITH NEW GPS MEASUREMENT HERE
        } else {
            // ESTIMATION WITHOUT NEW GPS MEASUREMENT HERE
        }
        now = current_time_microseconds();
        int sleep_time = (int)(max_sleep - (now - start_time));
        usleep(std::max(sleep_time, 0));
    }
}