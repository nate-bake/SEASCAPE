import time
import numpy as np
from core import shared_mem_helper_controller_1 as helper


def controller_loop(mem: helper.helper, max_sleep: float):
    # mem object is defined in 'core/shared_mem_helper_controller_1.py'
    # mem.read_xh() will return a list of all values from the xh vector specified in config.json
    # mem.read_servo() will return a list of current PWM values for each servo-rail channel.
    # mem.write_controller() will require a list of PWM values corresponding to each channel.
    # these outputs do not go directly to the servo-rail. they are instead handled by the RCIN_SERVO thread.

    xh_updates = 0
    while True:
        initial_time = time.time()
        [UPDATES, X, Y, Z, VT, ALPHA, BETA, PHI, THETA, PSI, P, Q, R] = mem.read_xh()
        [
            CHANNEL_0,
            CHANNEL_1,
            CHANNEL_2,
            CHANNEL_3,
            CHANNEL_4,
            CHANNEL_5,
            CHANNEL_6,
            CHANNEL_7,
            CHANNEL_8,
            CHANNEL_9,
            CHANNEL_10,
            CHANNEL_11,
            CHANNEL_12,
            CHANNEL_13,
        ] = mem.read_servo()
        if xh_updates == UPDATES:
            time.sleep(0.001)
            continue
        xh_updates = UPDATES
        # ======CONTROLLER CODE STARTS HERE===============================================
        # adjust channel values
        # =======CONTROLLER CODE STOPS HERE ======================================
        mem.write_controller(
            [
                CHANNEL_0,
                CHANNEL_1,
                CHANNEL_2,
                CHANNEL_3,
                CHANNEL_4,
                CHANNEL_5,
                CHANNEL_6,
                CHANNEL_7,
                CHANNEL_8,
                CHANNEL_9,
                CHANNEL_10,
                CHANNEL_11,
                CHANNEL_12,
                CHANNEL_13,
            ]
        )
        sleep_time = max_sleep - (time.time() - initial_time)
        time.sleep(max(sleep_time, 0))
