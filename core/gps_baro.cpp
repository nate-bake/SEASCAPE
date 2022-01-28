#include "air.h"

#include "Navio2/C++/Ublox.h"
#include "Navio2/C++/MS5611.h"


Ublox* initialize_gps(int milliseconds) {
    Ublox* gps = new Ublox();
    if (gps->testConnection()) {
        gps->configureSolutionRate(milliseconds);
    } else {
        printf("Ublox initialization FAILED\n");
        return nullptr;
    }
    return gps;
}

MS5611* initialize_baro() {
    MS5611* barometer = new MS5611();
    barometer->initialize();
    if (!barometer->testConnection()) {
        return NULL;
    }
    barometer->refreshPressure();
    return barometer;
}


void* gps_baro_loop(void* arguments) {
    thread_struct* args = (thread_struct*)arguments;
    double* array = args->array;
    const air_config* cfg = args->cfg;
    std::map<std::string, int> keys = cfg->keys;
    usleep(150000);
    Ublox* gps;
    MS5611* barometer;
    if (cfg->GPS_ENABLED) {
        printf("Initializing GPS.\n");
        gps = initialize_gps(200);
    }
    if (cfg->MS5611_ENABLED) {
        printf("Initializing Barometer.\t\t\t[MS5611]\n");
        barometer = initialize_baro();
    }
    usleep(500000);
    int max_sleep = hertz_to_microseconds(cfg->GPS_LOOP_RATE);
    uint64_t start_time, now;

    while (true) {
        start_time = current_time_microseconds();
        if (cfg->GPS_ENABLED && gps->decodeMessages(array, keys)) {
            array[keys["y_GPS_UPDATES"]]++;
        }
        if (cfg->MS5611_ENABLED) {
            barometer->refreshPressure();
            usleep(10000);
            barometer->readPressure();
            barometer->calculatePressureAndTemperature();
            array[keys["y_BARO_PRES"]] = barometer->getPressure();
        }
        now = current_time_microseconds();
        usleep(max_sleep - (now - start_time));
    }
}