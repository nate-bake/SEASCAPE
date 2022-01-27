#include "air.h"

void* estimation_loop(void* arguments) {
    usleep(5000000);
    thread_struct* args = (thread_struct*)arguments;
    double* array = args->array;
    const air_config* cfg = args->cfg;
    std::map<std::string, int> keys = cfg->keys;
    int imu = 0, gps = 0;
    int max_sleep = hertz_to_microseconds(cfg->ESTIMATION_LOOP_RATE);
    uint64_t start_time, now;

    while (true) {
        start_time = current_time_microseconds();
        if (imu == array[keys["y_IMU_UPDATES"]]) {
            continue;
        }
        imu = array[keys["y_IMU_UPDATES"]];

        ////////////////////////////////////////////////////////////////

        if (gps != array[keys["y_GPS_UPDATES"]]) {
            gps = array[keys["y_GPS_UPDATES"]];
            // ESTIMATION WITH NEW GPS MEASUREMENT HERE
        } else {
            // ESTIMATION WITHOUT NEW GPS MEASUREMENT HERE
        }

        ////////////////////////////////////////////////////////////////

        now = current_time_microseconds();
        int sleep_time = (int)(max_sleep - (now - start_time));
        usleep(std::max(sleep_time, 0));
    }
}