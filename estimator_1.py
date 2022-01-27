import time
import numpy as np
from core import shared_mem_helper_estimator_1 as helper


def estimator_loop(mem: helper.helper, max_sleep: float):
    # mem object is defined in 'core/shared_mem_helper_estimator_1.py'
    # mem.read_y() will return a list of all values defined in the y vector in config.json
    # mem.read_xh() will return a list of all values defined in the xh_1 vector in config.json
    # mem.write_xh() will require a list of all values defined in the xh_1 vector in config.json

    while True:
        t0 = time.time()

        # read latest sensor data
        # if you want to use IMU_2, be sure you have both IMUs enabled in config.json
        [
            NEW_GPS,
            IMU_1_AX,
            IMU_1_AY,
            IMU_1_AZ,
            IMU_1_GYRO_P,
            IMU_1_GYRO_Q,
            IMU_1_GYRO_R,
            IMU_1_MAG_X,
            IMU_1_MAG_Y,
            IMU_1_MAG_Z,
            IMU_2_AX,
            IMU_2_AY,
            IMU_2_AZ,
            IMU_2_GYRO_P,
            IMU_2_GYRO_Q,
            IMU_2_GYRO_R,
            IMU_2_MAG_X,
            IMU_2_MAG_Y,
            IMU_2_MAG_Z,
            BARO_PRES,
            GPS_POSN_LAT,
            GPS_POSN_LON,
            GPS_POSN_ALT,
            GPS_VEL_N,
            GPS_VEL_E,
            GPS_VEL_D,
            GPS_STATUS,
        ] = mem.read_y(use_calibrated_imu_data=True)

        # read previous xh data
        [X, Y, Z, VT, ALPHA, BETA, PHI, THETA, PSI, P, Q, R] = mem.read_xh()

        # \/ \/ \/ \/ \/ \/ \/ \/ \/  ESTIMATOR CODE STARTS HERE  \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ #

        if NEW_GPS:
            pass  # estimation with gps
        else:
            pass  # estimation without gps

        # /\ /\ /\ /\ /\ /\ /\ /\ /\  ESTIMATOR CODE STOPS HERE  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ #

        # publish new xh data
        mem.write_xh([X, Y, Z, VT, ALPHA, BETA, PHI, THETA, PSI, P, Q, R])

        # sleep to maintain frequency
        sleep_time = max_sleep - (time.time() - t0)
        time.sleep(max(sleep_time, 0))
