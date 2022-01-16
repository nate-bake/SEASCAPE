#include "air.h"

void* control_loop(void* arguments) {
    usleep(1500000);
    thread_struct* args = (thread_struct*)arguments;
    double* array = args->array;
    const air_config* cfg = args->cfg;
    std::map<std::string, int> keys = cfg->keys;
    std::string xh_vec = "xh_" + std::to_string(cfg->CONTROLLER_XH) + "_";
    int updates = 0;
    int max_sleep = hertz_to_microseconds(cfg->CONTROL_LOOP_RATE);
    uint64_t start_time, now;
    while (true) {
        start_time = current_time_microseconds();
        if (updates == array[keys[xh_vec + "UPDATES"]]) {
            continue;
        }
        updates = array[keys[xh_vec + "UPDATES"]];

        ////////////////////////////////////////////////////////////////

        array[keys["controller_0_THROTTLE"]] = array[keys["rcin_THROTTLE"]];
        array[keys["controller_0_ELEVATOR"]] = array[keys["rcin_ELEVATOR"]];
        array[keys["controller_0_AILERON"]] = array[keys["rcin_AILERON"]];
        array[keys["controller_0_FLAPS"]] = array[keys["rcin_FLAPS"]];
        array[keys["controller_0_RUDDER"]] = array[keys["rcin_RUDDER"]];

        ////////////////////////////////////////////////////////////////

        now = current_time_microseconds();
        int sleep_time = (int)(max_sleep - (now - start_time));
        usleep(std::max(sleep_time, 0));
    }
}