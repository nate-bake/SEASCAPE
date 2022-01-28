#include "air.h"

#include "Navio2/C++/LSM9DS1.h"
#include "Navio2/C++/MPU9250.h"
#include "Navio2/C++/ADC_Navio2.h"


InertialSensor* initialize_imu(std::string id, struct imu_calibration_profile* calibration_profile) {
    InertialSensor* imu;
    if (id == "LSM9DS1") {
        imu = new LSM9DS1();
    } else if (id == "MPU9250") {
        imu = new MPU9250();
    } else {
        return nullptr;
    }
    if (!imu->probe()) {
        printf("IMU initialization FAILED.\t\t[%s]\n", id);
        return nullptr;
    }
    printf("Initializing IMU.\t\t\t[%s]\n", id);
    imu->initialize();

    std::ifstream file("data/calibration/" + id + "_calibration.bin", std::ios::binary);
    printf("Loading IMU calibration profile.\t[%s]\n", id);

    char* memblock = new char[sizeof(double) * 9];
    file.read(memblock, sizeof(double) * 9);
    calibration_profile->offsets = (double*)memblock;

    char* memblock2 = new char[sizeof(double) * 9];
    file.read(memblock2, sizeof(double) * 9);
    calibration_profile->matrix = (double*)memblock2;

    if (!file.good()) {
        std::cout << "ERROR: Could not read 'data/calibration/" << id << "_calibration.bin'.\n";
        exit(-1);
    }
    file.close();

    imu->set_calibration_profile(calibration_profile);
    return imu;
}

ADC* initialize_adc() {
    ADC_Navio2* adc = new ADC_Navio2();
    printf("Initializing ADC.\n");
    adc->initialize();
    return adc;
}

int read_imu(InertialSensor* imu, double* array, std::map<std::string, int>& keys, int index) {
    imu->update();
    std::string prefix = "y_IMU_" + std::to_string(index) + "_";
    imu->read_accelerometer(array + keys[prefix + "AX_RAW"], array + keys[prefix + "AY_RAW"], array + keys[prefix + "AZ_RAW"]);
    imu->read_gyroscope(array + keys[prefix + "GYRO_P_RAW"], array + keys[prefix + "GYRO_Q_RAW"], array + keys[prefix + "GYRO_R_RAW"]);
    imu->read_magnetometer(array + keys[prefix + "MAG_X_RAW"], array + keys[prefix + "MAG_Y_RAW"], array + keys[prefix + "MAG_Z_RAW"]);
    imu->adjust();
    imu->read_accelerometer(array + keys[prefix + "AX_CALIB"], array + keys[prefix + "AY_CALIB"], array + keys[prefix + "AZ_CALIB"]);
    imu->read_gyroscope(array + keys[prefix + "GYRO_P_CALIB"], array + keys[prefix + "GYRO_Q_CALIB"], array + keys[prefix + "GYRO_R_CALIB"]);
    imu->read_magnetometer(array + keys[prefix + "MAG_X_CALIB"], array + keys[prefix + "MAG_Y_CALIB"], array + keys[prefix + "MAG_Z_CALIB"]);
    return 0;
}

int read_adc(ADC* adc, double* array, std::map<std::string, int>& keys) {
    array[keys["y_ADC_A0"]] = ((double)(adc->read(0))) / 1000;
    array[keys["y_ADC_A1"]] = ((double)(adc->read(1))) / 1000;
    array[keys["y_ADC_A2"]] = ((double)(adc->read(2))) / 1000;
    array[keys["y_ADC_A3"]] = ((double)(adc->read(3))) / 1000;
    array[keys["y_ADC_A4"]] = ((double)(adc->read(4))) / 1000;
    array[keys["y_ADC_A5"]] = ((double)(adc->read(5))) / 1000;
    // uint64_t us = current_time_microseconds();
    // double now = ((double)(us % 1000000000000)) / 1000000;
    // if (array[keys["y_ADC_TIME"]] != 0.) {
    //     array[keys["y_ADC_CONSUMED"]] += array[keys["y_ADC_A3"]] * 1000 * (now - array[keys["y_ADC_TIME"]]) / 3600;
    // }
    // array[keys["y_ADC_TIME"]] = now;
    return 0;
}


void* imu_loop(void* arguments) {
    thread_struct* args = (thread_struct*)arguments;
    double* array = args->array;
    const air_config* cfg = args->cfg;
    std::map<std::string, int> keys = cfg->keys;
    InertialSensor* lsm;
    InertialSensor* mpu;
    ADC* adc;
    if (cfg->LSM_ENABLED) {
        struct imu_calibration_profile calibration_profile;
        lsm = initialize_imu("LSM9DS1", &calibration_profile);
    }
    if (cfg->MPU_ENABLED) {
        struct imu_calibration_profile calibration_profile;
        mpu = initialize_imu("MPU9250", &calibration_profile);
    }
    if (cfg->ADC_ENABLED) {
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