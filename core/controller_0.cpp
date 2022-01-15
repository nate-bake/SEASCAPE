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
        // manual passthrough function
        array[keys["controller_0_CHANNEL_0"]] = array[keys["rcin_CHANNEL_0"]];
        array[keys["controller_0_CHANNEL_1"]] = array[keys["rcin_CHANNEL_1"]];
        array[keys["controller_0_CHANNEL_2"]] = array[keys["rcin_CHANNEL_2"]];
        array[keys["controller_0_CHANNEL_3"]] = array[keys["rcin_CHANNEL_3"]];
        array[keys["controller_0_CHANNEL_4"]] = array[keys["rcin_CHANNEL_4"]];
        array[keys["controller_0_CHANNEL_5"]] = array[keys["rcin_CHANNEL_5"]];
        now = current_time_microseconds();
        int sleep_time = (int)(max_sleep - (now - start_time));
        usleep(std::max(sleep_time, 0));
    }
}