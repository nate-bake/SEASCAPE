#include "air.h"

struct thread_struct {
    double* array;
    const air_config* cfg;
} thread_args;

uint64_t current_time_microseconds() {
    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

int hertz_to_microseconds(double hertz) {
    return (int)(1000000 / hertz);
}

float clip_pwm(const air_config* cfg, int pwm) {
    return (float)(std::max(cfg->MIN_PWM_OUT, std::min(pwm, cfg->MAX_PWM_OUT)));
}

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

InertialSensor* initialize_imu(std::string id = "LSM") {
    InertialSensor* imu;
    if (id == "LSM") {
        imu = new LSM9DS1();
    } else if (id == "MPU") {
        imu = new MPU9250();
    } else {
        return nullptr;
    }
    if (!imu->probe()) {
        printf("Sensor not enabled\n");
        return nullptr;
    }
    imu->initialize();
    return imu;
}

ADC* initialize_adc() {
    ADC_Navio2* adc = new ADC_Navio2();
    adc->initialize();
    return adc;
}

RCOutput* initialize_pwm(int freq) {
    RCOutput* pwm = new RCOutput_Navio2();
    for (int i = 0; i < 14; i++) {
        if (!pwm->initialize(i)) {
            return nullptr;
        }
        if (!pwm->set_frequency(i, freq)) {
            return nullptr;
        }
        if (!pwm->enable(i)) {
            return nullptr;
        }
    }
    return pwm;
}

int read_imu(InertialSensor* imu, double* array, std::map<std::string, int>& keys, int index = 1) {
    imu->update();
    std::string prefix = "y_IMU_" + std::to_string(index);
    imu->read_accelerometer(array + keys[prefix + "_AX"], array + keys[prefix + "_AY"], array + keys[prefix + "_AZ"]);
    imu->read_gyroscope(array + keys[prefix + "_GYRO_P"], array + keys[prefix + "_GYRO_Q"], array + keys[prefix + "_GYRO_Q"]);
    imu->read_magnetometer(array + keys[prefix + "_MAG_X"], array + keys[prefix + "_MAG_Y"], array + keys[prefix + "_MAG_Z"]);
    return 0;
}

int read_adc(ADC* adc, double* array, std::map<std::string, int>& keys) {
    array[keys["y_ADC_A0"]] = ((double)(adc->read(0))) / 1000;
    array[keys["y_ADC_A1"]] = ((double)(adc->read(1))) / 1000;
    array[keys["y_ADC_A2"]] = ((double)(adc->read(2))) / 1000;
    array[keys["y_ADC_A3"]] = ((double)(adc->read(3))) / 1000;
    array[keys["y_ADC_A4"]] = ((double)(adc->read(4))) / 1000;
    array[keys["y_ADC_A5"]] = ((double)(adc->read(5))) / 1000;
    uint64_t us = current_time_microseconds();
    double now = ((double)(us % 1000000000000)) / 1000000;
    if (array[keys["y_ADC_TIME"]] != 0.) {
        array[keys["y_ADC_CONSUMED"]] += array[keys["y_ADC_A3"]] * 1000 * (now - array[keys["y_ADC_TIME"]]) / 3600;
    }
    array[keys["y_ADC_TIME"]] = now;
    return 0;
}

int read_rcin(RCInput* rcin, double* array, std::map<std::string, int>& keys) {
    for (int i = 0; i < 14; i++) {
        array[keys["rcin_CHANNEL_" + std::to_string(i)]] = (double)(rcin->read(i));
    }
    return 0;
}

int write_servo(RCOutput* pwm, double* array, const air_config* cfg) {
    std::map<std::string, int> keys = cfg->keys;
    std::string controller_vec = "controller_" + std::to_string(cfg->SERVO_CONTROLLER) + "_";
    if (cfg->MANUAL_MODE_MIN <= array[keys["rcin_CHANNEL_" + std::to_string(cfg->FLIGHT_MODE_CHANNEL)]] <= cfg->MANUAL_MODE_MAX) {
        array[keys["servo_MODE_FLAG"]] = 0; // mode_flag: manual
        for (int i = 0; i < 14; i++) {
            float new_pwm = clip_pwm(cfg, (int)array[keys["rcin_CHANNEL_" + std::to_string(i)]]);
            pwm->set_duty_cycle(i, new_pwm);
            array[keys["servo_CHANNEL_" + std::to_string(i)]] = (double)new_pwm;
        }
    } else if (cfg->AUTO_MODE_MIN <= array[keys["rcin_CHANNEL_" + std::to_string(cfg->FLIGHT_MODE_CHANNEL)]] <= cfg->AUTO_MODE_MAX) {
        array[keys["servo_MODE_FLAG"]] = 1; //mode_flag: auto
        for (int i = 0; i < 14; i++) {
            float new_pwm = clip_pwm(cfg, (int)array[keys[controller_vec + "CHANNEL_" + std::to_string(i)]]);
            pwm->set_duty_cycle(i, new_pwm);
            array[keys["servo_CHANNEL_" + std::to_string(i)]] = (double)new_pwm;
        }
    } else if (cfg->SEMI_MODE_MIN <= array[keys["rcin_CHANNEL_" + std::to_string(cfg->FLIGHT_MODE_CHANNEL)]] <= cfg->SEMI_MODE_MAX) {
        array[keys["servo_MODE_FLAG"]] = 2; //mode_flag: semi-auto
        bool man_override = false;
        double el = array[keys["rcin_CHANNEL_" + std::to_string(cfg->ELEVATOR_CHANNEL)]];
        double al = array[keys["rcin_CHANNEL_" + std::to_string(cfg->AILERON_CHANNEL)]];
        if ((!(std::abs(1500 - el) > cfg->SEMI_DEADZONE)) || (!(std::abs(1500 - al) > cfg->SEMI_DEADZONE))) {
            man_override = true;
        }
        if (man_override) {
            for (int i = 0; i < 14; i++) {
                float new_pwm = clip_pwm(cfg, (int)array[keys["rcin_CHANNEL_" + std::to_string(i)]]);
                pwm->set_duty_cycle(i, new_pwm);
                array[keys["servo_CHANNEL_" + std::to_string(i)]] = (double)new_pwm;
            }
        } else {
            for (int i = 0; i < 14; i++) {
                float new_pwm = clip_pwm(cfg, (int)array[keys[controller_vec + "CHANNEL_" + std::to_string(i)]]);
                pwm->set_duty_cycle(i, new_pwm);
                array[keys["servo_CHANNEL_" + std::to_string(i)]] = (double)new_pwm;
            }
        }
    }
    return 0;
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
        printf("Initializing Barometer. [MS5611]\n");
        barometer = initialize_baro();
    }
    usleep(500000);
    int max_sleep = hertz_to_microseconds(cfg->GPS_LOOP_RATE);
    uint64_t start_time, now;
    while (true) {
        start_time = current_time_microseconds();
        if (cfg->GPS_ENABLED && gps->decodeMessages(array, keys)) {
            if (cfg->MS5611_ENABLED) {
                barometer->readPressure();
                barometer->calculatePressureAndTemperature();
                array[keys["y_BARO_PRES"]] = barometer->getPressure();
                if (array[keys["y_BARO_INIT"]] == 0) {
                    array[keys["y_BARO_INIT"]] = array[keys["y_BARO_PRES"]];
                }
                array[keys["y_GPS_UPDATES"]]++;
                barometer->refreshPressure();
            }
            now = current_time_microseconds();
            usleep(max_sleep - (now - start_time));
        } else if (!cfg->GPS_ENABLED && cfg->MS5611_ENABLED) {
            barometer->readPressure();
            barometer->calculatePressureAndTemperature();
            array[keys["y_BARO_PRES"]] = barometer->getPressure();
            if (array[keys["y_BARO_INIT"]] == 0) {
                array[keys["y_BARO_INIT"]] = array[keys["y_BARO_PRES"]];
            }
            barometer->refreshPressure();
            now = current_time_microseconds();
            int sleep_time = (int)(max_sleep - (now - start_time));
            usleep(std::max(sleep_time, 0));
        }
    }
}

void* imu_loop(void* arguments) {
    thread_struct* args = (thread_struct*)arguments;
    double* array = args->array;
    const air_config* cfg = args->cfg;
    std::map<std::string, int> keys = cfg->keys;
    InertialSensor* lsm;
    InertialSensor* mpu;
    ADC* adc;
    // NEED TO LOAD CALIBRATION PROFILES FOR IMUs
    // WILL PROBABLY BE EASIER IF WE MAKE AIR.CPP MORE MODULAR AND OBJECT-ORIENTED
    if (cfg->LSM_ENABLED) {
        printf("Initializing IMU. [LSM9DS1]\n");
        lsm = initialize_imu("LSM");
    }
    if (cfg->MPU_ENABLED) {
        printf("Initializing IMU. [MPU9250]\n");
        mpu = initialize_imu("MPU");
    }
    if (cfg->ADC_ENABLED) {
        printf("Initializing ADC.\n");
        adc = initialize_adc();
    }
    usleep(500000);
    int max_sleep = hertz_to_microseconds(cfg->IMU_LOOP_RATE);
    uint64_t start_time, now;
    while (true) {
        start_time = current_time_microseconds();
        if (cfg->LSM_ENABLED) {
            int index = 1;
            if (cfg->PRIMARY_IMU == "MPU9250") {
                index = 2;
            }
            read_imu(lsm, array, keys, index);
        }
        if (cfg->MPU_ENABLED) {
            int index = 2;
            if (!cfg->LSM_ENABLED || cfg->PRIMARY_IMU == "MPU9250") {
                index = 1;
            }
            read_imu(mpu, array, keys, index);
        }
        if (cfg->LSM_ENABLED || cfg->MPU_ENABLED) {
            array[keys["y_IMU_UPDATES"]]++;
        }
        if (cfg->ADC_ENABLED) {
            read_adc(adc, array, keys);
        }
        now = current_time_microseconds();
        int sleep_time = (int)(max_sleep - (now - start_time));
        usleep(std::max(sleep_time, 0));
    }
}

void* servo_loop(void* arguments) {
    thread_struct* args = (thread_struct*)arguments;
    double* array = args->array;
    const air_config* cfg = args->cfg;
    std::map<std::string, int> keys = cfg->keys;
    usleep(200000);
    usleep(50000);
    RCInput* rcin = new RCInput_Navio2();
    printf("Initializing RCIN.\n");
    rcin->initialize();
    printf("Initializing Servos.\n");
    RCOutput* pwm = initialize_pwm(cfg->PWM_FREQUENCY);
    if (!pwm) {
        printf("Failed to initialize servo-rail PWM. Could be lacking root privilege.\n");
    }
    usleep(500000);
    int max_sleep = hertz_to_microseconds(cfg->SERVO_LOOP_RATE);
    uint64_t start_time, now;
    while (true) {
        start_time = current_time_microseconds();
        read_rcin(rcin, array, keys);
        if (pwm) {
            write_servo(pwm, array, cfg);
        }
        now = current_time_microseconds();
        int sleep_time = (int)(max_sleep - (now - start_time));
        usleep(std::max(sleep_time, 0));
    }
}

void compute_average_bias(std::vector<std::vector<double>>& measured, std::vector<double>& avg) {
    avg.resize(3);
    for (int i = 0; i < measured.size(); i++) {
        avg[0] += measured[i][0];
        avg[1] += measured[i][1];
        avg[2] += measured[i][2];
    }
    avg[0] /= measured.size();
    avg[1] /= measured.size();
    avg[2] /= measured.size();
}

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
    printf("opening mavlink port. [/dev/tty/AMA0]\n");
    int max_sleep = hertz_to_microseconds(cfg->TELEMETRY_LOOP_RATE);
    while (true) {
        t = current_time_microseconds() - t0;
        mavlink_message_t msg0;
        int packed0 = mavlink_msg_heartbeat_pack(1, 255, &msg0, 1, 0, 4, 0, 0);
        int sent0 = port->write_message(msg0);
        // these values i am sending may not be correct but i just want to verify that mission planner displays them.
        mavlink_message_t msg30;
        int packed30 = mavlink_msg_attitude_pack(1, 255, &msg30, t, (float)array[keys["y_IMU_1_AX"]] / 20 * M_PI, (float)array[keys["y_IMU_1_AY"]] / 20 * M_PI, 0, 0, 0, 0);
        int sent30 = port->write_message(msg30);
        mavlink_message_t msg33;
        int packed33 = mavlink_msg_global_position_int_pack(1, 255, &msg33, t, 0, 0, 0, -1 * array[keys["xh_0_Z"]], array[keys["y_GPS_VEL_N"]], array[keys["y_GPS_VEL_E"]], array[keys["y_GPS_VEL_D"]], 0);
        int sent33 = port->write_message(msg33);
        mavlink_message_t msg24;
        int packed24 = mavlink_msg_gps_raw_int_pack(1, 255, &msg24, t, array[keys["y_GPS_STATUS"]], (int)array[keys["y_GPS_POSN_LAT"]], (int)array[keys["y_GPS_POSN_LON"]], (int)array[keys["y_GPS_POSN_ALT"]], 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
        int sent24 = port->write_message(msg24);
        // SYSTEM STATUS MSG #1 for battery
        // HUD MSG #74 for heading and speed
        now = current_time_microseconds() - t0;
        int sleep_time = (int)(max_sleep - (now - t));
        usleep(std::max(sleep_time, 0));
    }
}

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

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("ERROR: Please include config filepath as command-line argument.");
        exit(1);
    }
    const air_config cfg = air_config(argv[1]);
    double* array;

    key_t key = ftok(GETEKYDIR, PROJECTID);
    if (key < 0) {
        printf("ftok error");
        exit(1);
    }

    int shmid;
    shmid = shmget(key, SHMSIZE, IPC_CREAT | IPC_EXCL | 0664);
    if (shmid == -1) {
        if (errno == EEXIST) {
            printf("shared memory already exist\n");
            shmid = shmget(key, 0, 0);
            printf("reference shmid = %d\n", shmid);
        } else {
            perror("errno");
            printf("shmget error");
            exit(1);
        }
    }

    if ((array = (double*)shmat(shmid, 0, 0)) == (void*)-1) {
        if (shmctl(shmid, IPC_RMID, NULL) == -1) {
            printf("shmctl error");
            exit(1);
        } else {
            printf("Attach shared memory failed\n");
            printf("remove shared memory identifier successful\n");
        }

        printf("shmat error");
        exit(1);
    }

    memset(array, 0, SHMSIZE); // CLEAR SHARED VECTOR JUST IN CASE

    pthread_t imu_thread;
    pthread_t gps_baro_thread;
    pthread_t servo_thread;
    pthread_t estimation_thread;
    pthread_t control_thread;
    pthread_t telemetry_thread;

    thread_args.array = array;
    thread_args.cfg = &cfg;

    printf("\n");

    if (cfg.IMU_LOOP_ENABLED) {
        pthread_create(&imu_thread, NULL, &imu_loop, (void*)&thread_args);
    }
    if (cfg.GPS_LOOP_ENABLED) {
        pthread_create(&gps_baro_thread, NULL, &gps_baro_loop, (void*)&thread_args);
    }
    if (cfg.SERVO_LOOP_ENABLED) {
        pthread_create(&servo_thread, NULL, &servo_loop, (void*)&thread_args);
    }
    if (cfg.ESTIMATOR_ENABLED) {
        pthread_create(&estimation_thread, NULL, &estimation_loop, (void*)&thread_args);
    }
    if (cfg.CONTROLLER_ENABLED) {
        pthread_create(&control_thread, NULL, &control_loop, (void*)&thread_args);
    }
    if (cfg.TELEMETRY_LOOP_ENABLED) {
        pthread_create(&telemetry_thread, NULL, &telemetry_loop, (void*)&thread_args);
    }

    pthread_join(imu_thread, NULL);
    pthread_join(gps_baro_thread, NULL);
    pthread_join(servo_thread, NULL);
    pthread_join(estimation_thread, NULL);
    pthread_join(control_thread, NULL);
    pthread_join(telemetry_thread, NULL);

    return 0;
}