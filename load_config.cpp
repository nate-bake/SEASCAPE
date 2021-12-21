#include "air.h"

int load_config(std::map<std::string, double>& rates, std::map<std::string, int>& keys) {

    Json::Reader reader;
    Json::Value cfg;

    std::ifstream file("config.json");

    if (!reader.parse(file, cfg)) {
        std::cout << reader.getFormattedErrorMessages();
        exit(1);
    }

    auto rts = cfg["RATES"];
    if (rts["IMU_LOOP"].isNull()) {
        printf("no rate specified for: IMU_LOOP\nplease add to config.json.\n");
        exit(1);
    } else {
        rates.insert(std::pair<std::string, double>("IMU_LOOP", rts["IMU_LOOP"].asDouble()));
    }
    if (rts["GPS_BAROMETER_LOOP"].isNull()) {
        printf("no rate specified for: GPS_BAROMETER_LOOP\nplease add to config.json.\n");
        exit(1);
    } else {
        rates.insert(std::pair<std::string, double>("GPS_BAROMETER_LOOP", rts["GPS_BAROMETER_LOOP"].asDouble()));
    }
    if (rts["RCIN_SERVO_LOOP"].isNull()) {
        printf("no rate specified for: RCIN_SERVO_LOOP\nplease add to config.json.\n");
        exit(1);
    } else {
        rates.insert(std::pair<std::string, double>("RCIN_SERVO_LOOP", rts["RCIN_SERVO_LOOP"].asDouble()));
    }
    if (rts["BUILT-IN_ESTIMATOR"].isNull()) {
        printf("no rate specified for: BUILT-IN_ESTIMATOR\nplease add to config.json.\n");
        exit(1);
    } else {
        rates.insert(std::pair<std::string, double>("BUILT-IN_ESTIMATOR", rts["BUILT-IN_ESTIMATOR"].asDouble()));
    }
    if (rts["BUILT-IN_CONTROLLER"].isNull()) {
        printf("no rate specified for: BUILT-IN_CONTROLLER\nplease add to config.json.\n");
        exit(1);
    } else {
        rates.insert(std::pair<std::string, double>("BUILT-IN_CONTROLLER", rts["BUILT-IN_CONTROLLER"].asDouble()));
    }
    if (rts["TELEMETRY_LOOP"].isNull()) {
        printf("no rate specified for: TELEMETRY_LOOP\nplease add to config.json.\n");
        exit(1);
    } else {
        rates.insert(std::pair<std::string, double>("TELEMETRY_LOOP", rts["TELEMETRY_LOOP"].asDouble()));
    }

    int i = 1; // LEAVE INDEX 0 EMPTY FOR KEY NOT FOUND DEFAULT
    auto vectors = cfg["VECTORS"];

    for (int v = 0; v < vectors.size(); v++) {
        auto vector = vectors[v];
        if (vector["ENABLED"].asBool()) {
            auto ks = vector["KEYS"];
            for (int k = 0; k < ks.size(); k++) {
                if (!keys.insert(std::pair<std::string, int>(ks[k].asString(), i)).second) {
                    printf("DUPLICATE KEYS FOUND IN CONFIG FILE.\n");
                    std::exit(-1);
                }
                i++;
            }
        }
    }

    return 0;
}