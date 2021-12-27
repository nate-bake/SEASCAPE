import time
import numpy as np
from core import shared_mem_helper_estimator_1 as helper


def estimator_loop(mem: helper.helper, max_sleep: float):
    # mem object is defined in 'core/shared_mem_helper_estimator_1.py'
    # mem.read_y() will return a list of all values defined in the y vector in config.json
    # mem.read_xh() will return a list of all values defined in the xh_1 vector in config.json
    # mem.write_xh() will require a list of all values defined in the xh_1 vector in config.json

    gps = imu = 0  # used for detecting whether y vector has new data
    while True:
        initial_time = time.time()

        ############################### READ LATEST SENSOR DATA #################################
        [
            GPS_UPDATES,
            IMU_UPDATES,
            IMU_1_AX,
            IMU_1_AY,
            IMU_1_AZ,
            IMU_1_GYRO_P,
            IMU_1_GYRO_Q,
            IMU_1_GYRO_R,
            IMU_1_MAG_X,
            IMU_1_MAG_Y,
            IMU_1_MAG_Z,
            IMU_2_AX,  # if you want to use IMU_2, be sure you have both IMUs enabled in config.json
            IMU_2_AY,
            IMU_2_AZ,
            IMU_2_GYRO_P,
            IMU_2_GYRO_Q,
            IMU_2_GYRO_R,
            IMU_2_MAG_X,
            IMU_2_MAG_Y,
            IMU_2_MAG_Z,
            BARO_PRES,
            BARO_INIT,
            GPS_POSN_LAT,
            GPS_POSN_LON,
            GPS_POSN_ALT,
            GPS_VEL_N,
            GPS_VEL_E,
            GPS_VEL_D,
            GPS_STATUS,
            ADC_A0,
            ADC_A1,
            ADC_A2,
            ADC_A3,
            ADC_A4,
            ADC_A5,
            ADC_CONSUMED,
            ADC_TIME,
        ] = mem.read_y()

        ############################### READ PREVIOUS XH DATA ##################################

        [UPDATES, X, Y, Z, VT, ALPHA, BETA, PHI, THETA, PSI, P, Q, R] = mem.read_xh()

        ############################ CHECK IF SENSOR DATA IS NEW ###############################

        if imu == IMU_UPDATES:
            time.sleep(0.001)  # wait if no new data
            continue
        imu = IMU_UPDATES
        if gps != GPS_UPDATES:
            gps = GPS_UPDATES
            new_gps = True
        else:
            new_gps = False

        ############################# ESTIMATOR CODE STARTS HERE ###############################

        print(IMU_1_AX, IMU_2_AX)
        if new_gps:
            pass  # estimation with gps
        else:
            pass  # estaimation without gps

        ############################### PUBLISH NEW XH DATA ##################################

        mem.write_xh([UPDATES + 1, X, Y, Z, VT, ALPHA, BETA, PHI, THETA, PSI, P, Q, R])

        ############################ SLEEP TO MAINTAIN FREQUENCY #############################

        sleep_time = max_sleep - (time.time() - initial_time)
        time.sleep(max(sleep_time, 0))
