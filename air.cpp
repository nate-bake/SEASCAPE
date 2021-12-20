#include "air.h"

struct thread_struct
{
    double *array;
    std::map<std::string, int> keys;
} thread_args;

Ublox *initialize_gps(int milliseconds)
{
    // Ublox gps;
    Ublox *gps = new Ublox();
    if (gps->testConnection())
    {
        printf("Ublox test OK\n");
        gps->configureSolutionRate(milliseconds);
    }
    else
    {
        printf("Ublox initialization FAILED\n");
        return nullptr;
    }
    return gps;
}

MS5611 *initialize_baro()
{
    // MS5611 barometer;
    MS5611 *barometer = new MS5611();
    barometer->initialize();
    if (!barometer->testConnection())
        return NULL;
    barometer->refreshPressure();
    return barometer;
}

InertialSensor *initialize_imu()
{
    LSM9DS1 *imu = new LSM9DS1();
    // MPU9250* imu = new MPU9250();
    if (!imu->probe())
    {
        printf("Sensor not enabled\n");
        return nullptr;
    }
    if (!imu->initialize())
        return nullptr;
    return imu;
}

ADC *initialize_adc()
{
    ADC_Navio2 *adc = new ADC_Navio2();
    adc->initialize();
    return adc;
}

RCOutput *initialize_pwm()
{
    RCOutput *pwm = new RCOutput_Navio2();
    for (int i = 0; i < 6; i++)
    {
        if (!pwm->initialize(i))
            return nullptr;
        if (!pwm->set_frequency(i, 50))
            return nullptr;
        if (!pwm->enable(i))
            return nullptr;
    }
    return pwm;
}

int read_imu(InertialSensor *imu, double *array, std::map<std::string, int> &keys)
{
    imu->update();
    imu->read_accelerometer(array + keys["IMU_AX"], array + keys["IMU_AY"], array + keys["IMU_AZ"]);
    imu->read_gyroscope(array + keys["IMU_GYRO_P"], array + keys["IMU_GYRO_Q"], array + keys["IMU_GYRO_Q"]);
    imu->read_magnetometer(array + keys["IMU_MAG_X"], array + keys["IMU_MAG_Y"], array + keys["IMU_MAG_Z"]);
    return 0;
}

int read_adc(ADC *adc, double *array, std::map<std::string, int> &keys)
{
    array[keys["ADC_A0"]] = ((double)(adc->read(0))) / 1000;
    array[keys["ADC_A1"]] = ((double)(adc->read(1))) / 1000;
    array[keys["ADC_A2"]] = ((double)(adc->read(2))) / 1000;
    array[keys["ADC_A3"]] = ((double)(adc->read(3))) / 1000;
    array[keys["ADC_A4"]] = ((double)(adc->read(4))) / 1000;
    array[keys["ADC_A5"]] = ((double)(adc->read(5))) / 1000;
    uint64_t us = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    double now = ((double)(us % 1000000000000)) / 1000000;
    if (array[keys["ADC_TIME"]] != 0.)
        array[keys["ADC_CONSUMED"]] += array[keys["ADC_A3"]] * 1000 * (now - array[keys["ADC_TIME"]]) / 3600;
    array[keys["ADC_TIME"]] = now;
    return 0;
}

int read_rcin(RCInput *rcin, double *array, std::map<std::string, int> &keys)
{
    array[keys["RCIN_0"]] = (double)(rcin->read(0));
    array[keys["RCIN_1"]] = (double)(rcin->read(1));
    array[keys["RCIN_2"]] = (double)(rcin->read(2));
    array[keys["RCIN_3"]] = (double)(rcin->read(3));
    array[keys["RCIN_4"]] = (double)(rcin->read(4));
    array[keys["RCIN_5"]] = (double)(rcin->read(5));
    // SHOULD PROBABLY MAKE THIS MORE GENERAL TO SEE WHICH RCIN CHANNELS ARE IN CONFIG FILE
    return 0;
}

int write_servo(RCOutput *pwm, double *array, std::map<std::string, int> &keys)
{
    // THIS IS ALSO A BIT OF A MESS
    if (array[keys["RCIN_4"]] < 1250)
    {
        array[keys["MODE_FLAG"]] = 0; // mode_flag: manual
        pwm->set_duty_cycle(0, (int)array[keys["RCIN_0"]]);
        array[keys["SERVO_0"]] = array[keys["RCIN_0"]];
        pwm->set_duty_cycle(1, (int)array[keys["RCIN_1"]]);
        array[keys["SERVO_1"]] = array[keys["RCIN_1"]];
        pwm->set_duty_cycle(2, (int)array[keys["RCIN_2"]]);
        array[keys["SERVO_2"]] = array[keys["RCIN_2"]];
        pwm->set_duty_cycle(3, (int)array[keys["RCIN_3"]]);
        array[keys["SERVO_3"]] = array[keys["RCIN_3"]];
        pwm->set_duty_cycle(4, (int)array[keys["RCIN_4"]]);
        array[keys["SERVO_4"]] = array[keys["RCIN_4"]];
        pwm->set_duty_cycle(5, (int)array[keys["RCIN_5"]]);
        array[keys["SERVO_5"]] = array[keys["RCIN_5"]];
    }
    else if (array[keys["RCIN_4"]] > 1750)
    {
        array[keys["MODE_FLAG"]] = 1; //mode_flag: auto
        pwm->set_duty_cycle(0, (int)array[keys["CONTR_0"]]);
        array[keys["SERVO_0"]] = array[keys["CONTR_0"]];
        pwm->set_duty_cycle(1, (int)array[keys["CONTR_1"]]);
        array[keys["SERVO_1"]] = array[keys["CONTR_1"]];
        pwm->set_duty_cycle(2, (int)array[keys["CONTR_2"]]);
        array[keys["SERVO_2"]] = array[keys["CONTR_2"]];
        pwm->set_duty_cycle(3, (int)array[keys["CONTR_3"]]);
        array[keys["SERVO_3"]] = array[keys["CONTR_3"]];
        pwm->set_duty_cycle(4, (int)array[keys["CONTR_4"]]);
        array[keys["SERVO_4"]] = array[keys["CONTR_4"]];
        pwm->set_duty_cycle(5, (int)array[keys["CONTR_5"]]);
        array[keys["SERVO_5"]] = array[keys["CONTR_5"]];
    }
    else
    {
        array[keys["MODE_FLAG"]] = 2; //mode_flag: semi-auto
        bool man_override = false;
        if ((!(1.45 < array[keys["RCIN_2"]] < 1.55)) || (!(1.45 < array[keys["RCIN_3"]] < 1.55)))
        {
            man_override = true;
        }
        if (man_override)
        {
            pwm->set_duty_cycle(0, (int)array[keys["RCIN_0"]]);
            array[keys["SERVO_0"]] = array[keys["RCIN_0"]];
            pwm->set_duty_cycle(1, (int)array[keys["RCIN_1"]]);
            array[keys["SERVO_1"]] = array[keys["RCIN_1"]];
            pwm->set_duty_cycle(2, (int)array[keys["RCIN_2"]]);
            array[keys["SERVO_2"]] = array[keys["RCIN_2"]];
            pwm->set_duty_cycle(3, (int)array[keys["RCIN_3"]]);
            array[keys["SERVO_3"]] = array[keys["RCIN_3"]];
            pwm->set_duty_cycle(4, (int)array[keys["RCIN_4"]]);
            array[keys["SERVO_4"]] = array[keys["RCIN_4"]];
            pwm->set_duty_cycle(5, (int)array[keys["RCIN_5"]]);
            array[keys["SERVO_5"]] = array[keys["RCIN_5"]];
        }
        else
        {
            pwm->set_duty_cycle(0, (int)array[keys["CONTR_0"]]);
            array[keys["SERVO_0"]] = array[keys["CONTR_0"]];
            pwm->set_duty_cycle(1, (int)array[keys["CONTR_1"]]);
            array[keys["SERVO_1"]] = array[keys["CONTR_1"]];
            pwm->set_duty_cycle(2, (int)array[keys["CONTR_2"]]);
            array[keys["SERVO_2"]] = array[keys["CONTR_2"]];
            pwm->set_duty_cycle(3, (int)array[keys["CONTR_3"]]);
            array[keys["SERVO_3"]] = array[keys["CONTR_3"]];
            pwm->set_duty_cycle(4, (int)array[keys["CONTR_4"]]);
            array[keys["SERVO_4"]] = array[keys["CONTR_4"]];
            pwm->set_duty_cycle(5, (int)array[keys["CONTR_5"]]);
            array[keys["SERVO_5"]] = array[keys["CONTR_5"]];
        }
    }
    return 0;
}

void *gps_baro_loop(void *arguments)
{
    thread_struct *args = (thread_struct *)arguments;
    double *array = args->array;
    std::map<std::string, int> keys = args->keys;
    usleep(100000);
    printf("initializing gps.\n");
    usleep(50000);
    printf("initializing barometer. [MS5611]\n");
    Ublox *gps = initialize_gps(200);
    MS5611 *barometer = initialize_baro();
    usleep(500000);
    while (true)
    {
        if (gps->decodeMessages(array, keys))
        {
            barometer->readPressure();
            barometer->calculatePressureAndTemperature();
            array[keys["BARO_PRES"]] = barometer->getPressure();
            if (array[keys["BARO_INIT"]] == 0)
                array[keys["BARO_INIT"]] = array[keys["BARO_PRES"]];
            array[keys["GPS_UPDATES"]]++;
            barometer->refreshPressure();
            usleep(160000); // sleep 160 ms for 5 hz
        }
    }
}

void *imu_loop(void *arguments)
{
    thread_struct *args = (thread_struct *)arguments;
    double *array = args->array;
    std::map<std::string, int> keys = args->keys;
    printf("initializing imu. [LSM9DS1]\n");
    usleep(50000);
    printf("initializing adc.\n");
    InertialSensor *imu = initialize_imu();
    ADC *adc = initialize_adc();
    usleep(500000);
    while (true)
    {
        read_imu(imu, array, keys);
        array[keys["[IMU_UPDATES"]]++;
        read_adc(adc, array, keys);
        usleep(4000);
    }
}

void *servo_loop(void *arguments)
{
    thread_struct *args = (thread_struct *)arguments;
    double *array = args->array;
    std::map<std::string, int> keys = args->keys;
    usleep(200000);
    printf("initializing rcin.\n");
    usleep(50000);
    printf("initializing servos.\n");
    RCInput *rcin = new RCInput_Navio2();
    rcin->initialize();
    RCOutput *pwm = initialize_pwm();
    usleep(500000);
    while (true)
    {
        read_rcin(rcin, array, keys);
        if (pwm)
            write_servo(pwm, array, keys);
        usleep(17250);
    }
}

void compute_average_bias(std::vector<std::vector<double>> &measured, std::vector<double> &avg)
{
    avg.resize(3);
    for (int i = 0; i < measured.size(); i++)
    {
        avg[0] += measured[i][0];
        avg[1] += measured[i][1];
        avg[2] += measured[i][2];
    }
    avg[0] /= measured.size();
    avg[1] /= measured.size();
    avg[2] /= measured.size();
}

void *telemetry_loop(void *arguments)
{
    usleep(300000);
    uint64_t t0 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    uint32_t t;
    thread_struct *args = (thread_struct *)arguments;
    double *array = args->array;
    std::map<std::string, int> keys = args->keys;
    Generic_Port *port;
    port = new Serial_Port("/dev/ttyAMA0", 57600);
    port->start();
    printf("opening mavlink port. [/dev/tty/AMA0]\n");
    while (true)
    {
        usleep(500000);
        t = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() - t0;
        mavlink_message_t msg0;
        int packed0 = mavlink_msg_heartbeat_pack(1, 255, &msg0, 1, 0, 4, 0, 0);
        int sent0 = port->write_message(msg0);
        // these values i am sending may not be correct but i just want to verify that mission planner displays them.
        mavlink_message_t msg30;
        int packed30 = mavlink_msg_attitude_pack(1, 255, &msg30, t, (float)array[keys["IMU_AX"]] / 20 * M_PI, (float)array[keys["IMU_AY"]] / 20 * M_PI, 0, 0, 0, 0);
        int sent30 = port->write_message(msg30);
        mavlink_message_t msg33;
        int packed33 = mavlink_msg_global_position_int_pack(1, 255, &msg33, t, 0, 0, 0, -1 * array[keys["XH_Z"]], array[keys["GPS_VEL_N"]], array[keys["GPS_VEL_E"]], array[keys["GPS_VEL_D"]], 0);
        int sent33 = port->write_message(msg33);
        mavlink_message_t msg24;
        int packed24 = mavlink_msg_gps_raw_int_pack(1, 255, &msg24, t, array[keys["GPS_STATUS"]], (int)array[keys["GPS_POSN_LAT"]], (int)array[keys["GPS_POSN_LON"]], (int)array[keys["GPS_POSN_ALT"]], 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
        int sent24 = port->write_message(msg24);
        // SYSTEM STATUS MSG #1 for battery
        // HUD MSG #74 for heading and speed
    }
}

void *estimator_loop(void *arguments)
{
    usleep(1500000);
    thread_struct *args = (thread_struct *)arguments;
    double *array = args->array;
    std::map<std::string, int> keys = args->keys;
    int imu = 0, gps = 0;

    std::vector<std::vector<double>> accel, gyro, mag;
    uint64_t t1 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    uint64_t t2 = t1;
    printf("\nmeasuring imu bias. [keep aircraft level]\n");
    char chars[] = {'-', '-', '\\', '\\', '|', '|', '/', '/'};
    unsigned int i = 0;
    while ((t2 - t1) < 7000)
    {
        if (i++ % 2 == 0)
            printf("\r%c ", chars[i % sizeof(chars)]);
        fflush(stdout);
        t2 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        if (imu == array[keys["IMU_UPDATES"]])
            continue;
        imu = array[keys["IMU_UPDATES"]];
        accel.push_back({array[keys["IMU_AX"]], array[keys["IMU_AY"]], array[keys["IMU_AZ"]]});
        gyro.push_back({array[keys["IMU_GYRO_P"]], array[keys["IMU_GYRO_Q"]], array[keys["IMU_GYRO_R"]]});
        mag.push_back({array[keys["IMU_MAG_X"]], array[keys["IMU_MAG_Y"]], array[keys["IMU_MAG_Z"]]});
        usleep(50000);
    }
    printf("\rbias measurement complete.\n\n");
    std::vector<double> accel_bias, gyro_bias, mag_bias;
    compute_average_bias(accel, accel_bias);
    compute_average_bias(gyro, gyro_bias);
    compute_average_bias(mag, mag_bias);
    // std::cout << accel_bias.at(0) << std::endl;

    while (true)
    {
        uint64_t start_time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        bool new_gps = false;
        if (imu == array[keys["IMU_UPDATES"]])
            continue;
        imu = array[keys["IMU_UPDATES"]];
        if (gps != array[keys["GPS_UPDATES"]])
        {
            gps = array[keys["GPS_UPDATES"]];
            new_gps = true;
            // printf("new gps\n");
            // ESTIMATION WITH NEW GPS MEASUREMENT HERE
        }
        else
        {
            // printf("no new gps\n");
            // ESTIMATION WITHOUT NEW GPS MEASUREMENT HERE
        }
        uint64_t now = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        usleep(10000 - (now - start_time));
    }
}

void *controller_loop(void *arguments)
{
    usleep(1500000);
    thread_struct *args = (thread_struct *)arguments;
    double *array = args->array;
    std::map<std::string, int> keys = args->keys;
    int updates = 0;
    while (true)
    {
        uint64_t start_time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        if (updates == array[keys["XH_UPDATES"]])
            continue;
        updates = array[keys["XH_UPDATES"]];
        // manual passthrough function
        array[keys["CONTR_0"]] = array[keys["RCIN_0"]];
        array[keys["CONTR_1"]] = array[keys["RCIN_1"]];
        array[keys["CONTR_2"]] = array[keys["RCIN_2"]];
        array[keys["CONTR_3"]] = array[keys["RCIN_3"]];
        // array[keys["CONTR_4"]] = array[keys["RCIN_4"]];
        array[keys["CONTR_5"]] = array[keys["RCIN_5"]];
        uint64_t now = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        usleep(10000 - (now - start_time));
    }
}

int main(int argc, char *argv[])
{
    std::map<std::string, int> keys;
    double *array;

    load_config(keys);

    key_t key = ftok(GETEKYDIR, PROJECTID);
    if (key < 0)
    {
        printf("ftok error");
        exit(1);
    }

    int shmid;
    shmid = shmget(key, SHMSIZE, IPC_CREAT | IPC_EXCL | 0664);
    if (shmid == -1)
    {
        if (errno == EEXIST)
        {
            printf("shared memory already exist\n");
            shmid = shmget(key, 0, 0);
            printf("reference shmid = %d\n", shmid);
        }
        else
        {
            perror("errno");
            printf("shmget error");
            exit(1);
        }
    }

    if ((array = (double *)shmat(shmid, 0, 0)) == (void *)-1)
    {
        if (shmctl(shmid, IPC_RMID, NULL) == -1)
        {
            printf("shmctl error");
            exit(1);
        }
        else
        {
            printf("Attach shared memory failed\n");
            printf("remove shared memory identifier successful\n");
        }

        printf("shmat error");
        exit(1);
    }

    pthread_t imu_thread;
    pthread_t gps_baro_thread;
    pthread_t servo_thread;
    pthread_t estimator_thread;
    pthread_t controller_thread;
    pthread_t telemetry_thread;

    thread_args.array = array;
    thread_args.keys = keys;

    printf("\n");
    pthread_create(&imu_thread, NULL, &imu_loop, (void *)&thread_args);
    pthread_create(&gps_baro_thread, NULL, &gps_baro_loop, (void *)&thread_args);
    pthread_create(&servo_thread, NULL, &servo_loop, (void *)&thread_args);
    pthread_create(&estimator_thread, NULL, &estimator_loop, (void *)&thread_args);
    pthread_create(&controller_thread, NULL, &controller_loop, (void *)&thread_args);
    pthread_create(&telemetry_thread, NULL, &telemetry_loop, (void *)&thread_args);

    printf("Enter to exit\n");
    getchar();

    printf("%f\n", array[0]);

    if (shmdt(array) < 0)
    {
        printf("shmdt error");
        exit(1);
    }

    if (shmctl(shmid, IPC_RMID, NULL) == -1)
    {
        printf("shmctl error");
        exit(1);
    }
    else
    {
        printf("Finally\n");
        printf("remove shared memory identifier successful\n");
    }

    return 0;
}