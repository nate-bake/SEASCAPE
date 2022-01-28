#include "air.h"

#include "mavlink/common/mavlink.h"
#include "Navio2/serial_port.h"

void* telemetry_loop(void* arguments) {
    usleep(300000);
    uint64_t t0 = current_time_microseconds();
    uint64_t t, now;
    thread_struct* args = (thread_struct*)arguments;
    double* array = args->array;
    const air_config* cfg = args->cfg;
    std::map<std::string, int> keys = cfg->keys;
    Generic_Port* port;
    port = new Serial_Port("/dev/ttyAMA0", 57600);
    port->start();
    printf("Opening mavlink port.\t[/dev/tty/AMA0]\n");
    int max_sleep = hertz_to_microseconds(cfg->TELEMETRY_LOOP_RATE);

    while (true) {
        t = current_time_microseconds() - t0;
        mavlink_message_t msg0;
        int packed0 = mavlink_msg_heartbeat_pack(1, 255, &msg0, 1, 0, 4, 0, 0);
        int sent0 = port->write_message(msg0);
        // the transmitted values may not be correct but i just want to verify that mission planner displays them.
        mavlink_message_t msg30;
        int packed30 = mavlink_msg_attitude_pack(1, 255, &msg30, t, (float)array[keys["y_IMU_1_AX_CALIB"]] / 20 * M_PI, (float)array[keys["y_IMU_1_AY_CALIB"]] / 20 * M_PI, 0, 0, 0, 0);
        int sent30 = port->write_message(msg30);
        mavlink_message_t msg33;
        int packed33 = mavlink_msg_global_position_int_pack(1, 255, &msg33, t, 0, 0, 0, -1 * array[keys["xh_0_Z"]], array[keys["y_GPS_VEL_N"]], array[keys["y_GPS_VEL_E"]], array[keys["y_GPS_VEL_D"]], 0);
        int sent33 = port->write_message(msg33);
        mavlink_message_t msg24;
        int packed24 = mavlink_msg_gps_raw_int_pack(1, 255, &msg24, t, array[keys["y_GPS_STATUS"]], (int)(array[keys["y_GPS_POSN_LAT"]]), (int)(array[keys["y_GPS_POSN_LON"]]), (int)array[keys["y_GPS_POSN_ALT"]], 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
        int sent24 = port->write_message(msg24);
        // SYSTEM STATUS MSG #1 for battery
        // HUD MSG #74 for heading and speed
        now = current_time_microseconds() - t0;
        int sleep_time = (int)(max_sleep - (now - t));
        usleep(std::max(sleep_time, 0));
    }
}