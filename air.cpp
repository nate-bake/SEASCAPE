#include "air.h"

struct thread_struct {
    double* array;
    std::map<std::string, int> keys;
    std::map<std::string, double> rates;
} thread_args;

uint64_t current_time_microseconds() {
    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

int hertz_to_microseconds(double hertz) {
    return (int)(1000000 / hertz);
}

Ublox* initialize_gps(int milliseconds) {
    Ublox* gps = new Ublox();
    if (gps->testConnection()) {
        printf("Ublox test OK\n");
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

RCOutput* initialize_pwm() {
    RCOutput* pwm = new RCOutput_Navio2();
    for (int i = 0; i < 6; i++) {
        if (!pwm->initialize(i)) {
            return nullptr;
        }
        if (!pwm->set_frequency(i, 50)) {
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
    std::string prefix = "IMU_" + std::to_string(index);
    imu->read_accelerometer(array + keys[prefix + "_AX"], array + keys[prefix + "_AY"], array + keys[prefix + "_AZ"]);
    imu->read_gyroscope(array + keys[prefix + "_GYRO_P"], array + keys[prefix + "_GYRO_Q"], array + keys[prefix + "_GYRO_Q"]);
    imu->read_magnetometer(array + keys[prefix + "_MAG_X"], array + keys[prefix + "_MAG_Y"], array + keys[prefix + "_MAG_Z"]);
    return 0;
}

int read_adc(ADC* adc, double* array, std::map<std::string, int>& keys) {
    array[keys["ADC_A0"]] = ((double)(adc->read(0))) / 1000;
    array[keys["ADC_A1"]] = ((double)(adc->read(1))) / 1000;
    array[keys["ADC_A2"]] = ((double)(adc->read(2))) / 1000;
    array[keys["ADC_A3"]] = ((double)(adc->read(3))) / 1000;
    array[keys["ADC_A4"]] = ((double)(adc->read(4))) / 1000;
    array[keys["ADC_A5"]] = ((double)(adc->read(5))) / 1000;
    uint64_t us = current_time_microseconds();
    double now = ((double)(us % 1000000000000)) / 1000000;
    if (array[keys["ADC_TIME"]] != 0.) {
        array[keys["ADC_CONSUMED"]] += array[keys["ADC_A3"]] * 1000 * (now - array[keys["ADC_TIME"]]) / 3600;
    }
    array[keys["ADC_TIME"]] = now;
    return 0;
}

int read_rcin(RCInput* rcin, double* array, std::map<std::string, int>& keys) {
    array[keys["RCIN_0"]] = (double)(rcin->read(0));
    array[keys["RCIN_1"]] = (double)(rcin->read(1));
    array[keys["RCIN_2"]] = (double)(rcin->read(2));
    array[keys["RCIN_3"]] = (double)(rcin->read(3));
    array[keys["RCIN_4"]] = (double)(rcin->read(4));
    array[keys["RCIN_5"]] = (double)(rcin->read(5));
    // SHOULD PROBABLY MAKE THIS MORE GENERAL TO SEE WHICH RCIN CHANNELS ARE IN CONFIG FILE
    return 0;
}

int write_servo(RCOutput* pwm, double* array, std::map<std::string, int>& keys) {
    // THIS IS ALSO A BIT OF A MESS
    if (array[keys["RCIN_4"]] < 1250) {
        array[keys["MODE_FLAG"]] = 0; // mode_flag: manual
        for (int i = 0; i < 14; i++) {
            pwm->set_duty_cycle(i, (int)array[keys["RCIN_" + std::to_string(i)]]);
            array[keys["SERVO_" + std::to_string(i)]] = array[keys["RCIN_" + std::to_string(i)]];
        }
    } else if (array[keys["RCIN_4"]] > 1750) {
        array[keys["MODE_FLAG"]] = 1; //mode_flag: auto
        for (int i = 0; i < 14; i++) {
            pwm->set_duty_cycle(i, (int)array[keys["CONTROLLER_" + std::to_string(i)]]);
            array[keys["SERVO_" + std::to_string(i)]] = array[keys["CONTROLLER_" + std::to_string(i)]];
        }
    } else {
        array[keys["MODE_FLAG"]] = 2; //mode_flag: semi-auto
        bool man_override = false;
        if ((!(1.45 < array[keys["RCIN_2"]] < 1.55)) || (!(1.45 < array[keys["RCIN_3"]] < 1.55))) {
            man_override = true;
        }
        if (man_override) {
            for (int i = 0; i < 14; i++) {
                pwm->set_duty_cycle(i, (int)array[keys["RCIN_" + std::to_string(i)]]);
                array[keys["SERVO_" + std::to_string(i)]] = array[keys["RCIN_" + std::to_string(i)]];
            }
        } else {
            for (int i = 0; i < 14; i++) {
                pwm->set_duty_cycle(i, (int)array[keys["CONTROLLER_" + std::to_string(i)]]);
                array[keys["SERVO_" + std::to_string(i)]] = array[keys["CONTROLLER_" + std::to_string(i)]];
            }
        }
    }
    return 0;
}

void* gps_baro_loop(void* arguments) {
    thread_struct* args = (thread_struct*)arguments;
    double* array = args->array;
    std::map<std::string, int> keys = args->keys;
    std::map<std::string, double> rates = args->rates;
    usleep(100000);
    printf("initializing gps.\n");
    usleep(50000);
    printf("initializing barometer. [MS5611]\n");
    Ublox* gps = initialize_gps(200);
    MS5611* barometer = initialize_baro();
    usleep(500000);
    int max_sleep = hertz_to_microseconds(rates["GPS_BAROMETER_LOOP"]);
    uint64_t start_time, now;
    while (true) {
        start_time = current_time_microseconds();
        if (gps->decodeMessages(array, keys)) {
            barometer->readPressure();
            barometer->calculatePressureAndTemperature();
            array[keys["BARO_PRES"]] = barometer->getPressure();
            if (array[keys["BARO_INIT"]] == 0) {
                array[keys["BARO_INIT"]] = array[keys["BARO_PRES"]];
            }
            array[keys["GPS_UPDATES"]]++;
            barometer->refreshPressure();
            now = current_time_microseconds();
            usleep(max_sleep - (now - start_time));
        }
    }
}

void* imu_loop(void* arguments) {
    thread_struct* args = (thread_struct*)arguments;
    double* array = args->array;
    std::map<std::string, int> keys = args->keys;
    std::map<std::string, double> rates = args->rates;
    printf("initializing imus.\n");
    usleep(50000);
    printf("initializing adc.\n");
    InertialSensor* lsm = initialize_imu("LSM");
    InertialSensor* mpu = initialize_imu("MPU");
    ADC* adc = initialize_adc();
    usleep(500000);
    int max_sleep = hertz_to_microseconds(rates["IMU_LOOP"]);
    uint64_t start_time, now;
    while (true) {
        start_time = current_time_microseconds();
        read_imu(lsm, array, keys, 1);
        read_imu(mpu, array, keys, 2);
        array[keys["IMU_UPDATES"]]++;
        read_adc(adc, array, keys);
        now = current_time_microseconds();
        usleep(max_sleep - (now - start_time));
    }
}

void* servo_loop(void* arguments) {
    thread_struct* args = (thread_struct*)arguments;
    double* array = args->array;
    std::map<std::string, int> keys = args->keys;
    std::map<std::string, double> rates = args->rates;
    usleep(200000);
    printf("initializing rcin.\n");
    usleep(50000);
    printf("initializing servos.\n");
    RCInput* rcin = new RCInput_Navio2();
    rcin->initialize();
    RCOutput* pwm = initialize_pwm();
    usleep(500000);
    int max_sleep = hertz_to_microseconds(rates["RCIN_SERVO_LOOP"]);
    uint64_t start_time, now;
    while (true) {
        start_time = current_time_microseconds();
        read_rcin(rcin, array, keys);
        if (pwm) {
            write_servo(pwm, array, keys);
        }
        now = current_time_microseconds();
        usleep(max_sleep - (now - start_time));
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
    std::map<std::string, int> keys = args->keys;
    std::map<std::string, double> rates = args->rates;
    Generic_Port* port;
    port = new Serial_Port("/dev/ttyAMA0", 57600);
    port->start();
    printf("opening mavlink port. [/dev/tty/AMA0]\n");
    int max_sleep = hertz_to_microseconds(rates["TELEMETRY_LOOP"]);
    while (true) {
        t = current_time_microseconds() - t0;
        mavlink_message_t msg0;
        int packed0 = mavlink_msg_heartbeat_pack(1, 255, &msg0, 1, 0, 4, 0, 0);
        int sent0 = port->write_message(msg0);
        // these values i am sending may not be correct but i just want to verify that mission planner displays them.
        mavlink_message_t msg30;
        int packed30 = mavlink_msg_attitude_pack(1, 255, &msg30, t, (float)array[keys["IMU_1_AX"]] / 20 * M_PI, (float)array[keys["IMU_1_AY"]] / 20 * M_PI, 0, 0, 0, 0);
        int sent30 = port->write_message(msg30);
        mavlink_message_t msg33;
        int packed33 = mavlink_msg_global_position_int_pack(1, 255, &msg33, t, 0, 0, 0, -1 * array[keys["XH_Z"]], array[keys["GPS_VEL_N"]], array[keys["GPS_VEL_E"]], array[keys["GPS_VEL_D"]], 0);
        int sent33 = port->write_message(msg33);
        mavlink_message_t msg24;
        int packed24 = mavlink_msg_gps_raw_int_pack(1, 255, &msg24, t, array[keys["GPS_STATUS"]], (int)array[keys["GPS_POSN_LAT"]], (int)array[keys["GPS_POSN_LON"]], (int)array[keys["GPS_POSN_ALT"]], 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
        int sent24 = port->write_message(msg24);
        // SYSTEM STATUS MSG #1 for battery
        // HUD MSG #74 for heading and speed
        now = current_time_microseconds() - t0;
        usleep(max_sleep - (now - t));
    }
}

void* estimator_loop(void* arguments) {
    usleep(1500000);
    thread_struct* args = (thread_struct*)arguments;
    double* array = args->array;
    std::map<std::string, int> keys = args->keys;
    std::map<std::string, double> rates = args->rates;
    int imu = 0, gps = 0;

    std::vector<std::vector<double>> accel, gyro, mag;
    uint64_t t1 = current_time_microseconds();
    uint64_t t2 = t1;
    printf("\nmeasuring imu bias. [keep aircraft level]\n");
    char chars[] = { '-', '-', '\\', '\\', '|', '|', '/', '/' };
    unsigned int i = 0;
    while ((t2 - t1) < 7000) {
        if (i % 2 == 0) {
            printf("\r%c ", chars[i % sizeof(chars)]);
        }
        fflush(stdout);
        t2 = current_time_microseconds();
        if (imu == array[keys["IMU_UPDATES"]]) {
            continue;
        }
        imu = array[keys["IMU_UPDATES"]];
        accel.push_back({ array[keys["IMU_1_AX"]], array[keys["IMU_1_AY"]], array[keys["IMU_1_AZ"]] });
        gyro.push_back({ array[keys["IMU_1_GYRO_P"]], array[keys["IMU_1_GYRO_Q"]], array[keys["IMU_1_GYRO_R"]] });
        mag.push_back({ array[keys["IMU_1_MAG_X"]], array[keys["IMU_1_MAG_Y"]], array[keys["IMU_1_MAG_Z"]] });
        usleep(50000);
        i++;
    }
    printf("\rbias measurement complete.\n\n");
    std::vector<double> accel_bias, gyro_bias, mag_bias;
    compute_average_bias(accel, accel_bias);
    compute_average_bias(gyro, gyro_bias);
    compute_average_bias(mag, mag_bias);
    // std::cout << accel_bias.at(0) << std::endl;

    int max_sleep = hertz_to_microseconds(rates["BUILT-IN_ESTIMATOR"]);
    uint64_t start_time, now;
    while (true) {
        start_time = current_time_microseconds();
        bool new_gps = false;
        if (imu == array[keys["IMU_UPDATES"]]) {
            continue;
        }
        imu = array[keys["IMU_UPDATES"]];
        if (gps != array[keys["GPS_UPDATES"]]) {
            gps = array[keys["GPS_UPDATES"]];
            new_gps = true;
            // printf("new gps\n");
            // ESTIMATION WITH NEW GPS MEASUREMENT HERE
        } else {
            // printf("no new gps\n");
            // ESTIMATION WITHOUT NEW GPS MEASUREMENT HERE
        }
        now = current_time_microseconds();
        usleep(max_sleep - (now - start_time));
    }
}

void* controller_loop(void* arguments) {
    usleep(1500000);
    thread_struct* args = (thread_struct*)arguments;
    double* array = args->array;
    std::map<std::string, int> keys = args->keys;
    std::map<std::string, double> rates = args->rates;
    int updates = 0;
    int max_sleep = hertz_to_microseconds(rates["BUILT-IN_CONTROLLER"]);
    uint64_t start_time, now;
    while (true) {
        start_time = current_time_microseconds();
        if (updates == array[keys["XH_UPDATES"]]) {
            continue;
        }
        updates = array[keys["XH_UPDATES"]];
        // manual passthrough function
        array[keys["CONTROLLER_0"]] = array[keys["RCIN_0"]];
        array[keys["CONTROLLER_1"]] = array[keys["RCIN_1"]];
        array[keys["CONTROLLER_2"]] = array[keys["RCIN_2"]];
        array[keys["CONTROLLER_3"]] = array[keys["RCIN_3"]];
        array[keys["CONTROLLER_4"]] = array[keys["RCIN_4"]];
        array[keys["CONTROLLER_5"]] = array[keys["RCIN_5"]];
        now = current_time_microseconds();
        usleep(max_sleep - (now - start_time));
    }
}

int main(int argc, char* argv[]) {
    std::map<std::string, double> rates;
    std::map<std::string, int> keys;
    double* array;

    load_config(rates, keys);

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

    memset(array, 0, 2048); // CLEAR SHARED VECTOR JUST IN CASE

    pthread_t imu_thread;
    pthread_t gps_baro_thread;
    pthread_t servo_thread;
    pthread_t estimator_thread;
    pthread_t controller_thread;
    pthread_t telemetry_thread;

    thread_args.array = array;
    thread_args.keys = keys;
    thread_args.rates = rates;

    printf("\n");

    if (rates["IMU_LOOP"] > 0) {
        pthread_create(&imu_thread, NULL, &imu_loop, (void*)&thread_args);
    }
    if (rates["GPS_BAROMETER_LOOP"] > 0) {
        pthread_create(&gps_baro_thread, NULL, &gps_baro_loop, (void*)&thread_args);
    }
    if (rates["RCIN_SERVO_LOOP"] > 0) {
        pthread_create(&servo_thread, NULL, &servo_loop, (void*)&thread_args);
    }
    if (rates["BUILT-IN_ESTIMATOR"] > 0) {
        pthread_create(&estimator_thread, NULL, &estimator_loop, (void*)&thread_args);
    }
    if (rates["BUILT-IN_CONTROLLER"] > 0) {
        pthread_create(&controller_thread, NULL, &controller_loop, (void*)&thread_args);
    }
    if (rates["TELEMETRY_LOOP"] > 0) {
        pthread_create(&telemetry_thread, NULL, &telemetry_loop, (void*)&thread_args);
    }

    usleep(10000000);
    printf("Enter to exit\n");
    getchar();

    if (shmdt(array) < 0) {
        printf("shmdt error");
        exit(1);
    }

    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        printf("shmctl error");
        exit(1);
    } else {
        printf("remove shared memory identifier successful\n");
    }

    return 0;
}