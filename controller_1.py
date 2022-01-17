import time
import numpy as np
from core import shared_mem_helper_controller_1 as shm


def controller_loop(mem: shm.helper, max_sleep: float):
    # mem object is defined in 'core/shared_mem_helper_controller_1.py'
    # mem.read_xh() will return a list of all values from the xh vector specified in config.json
    # mem.read_rcin() will update the object's list of current RC inputs from each channel.
    # ---- these values can be addressed by channel, such as mem.rcin[3]
    # ---- or by channel standard channel names, such as mem.rcin.throttle or mem.rcin.flaps
    # mem.read_servo() will update the object's list of current PWM values for each servo-rail channel.
    # ---- these values can be addressed by channel, such as mem.servos[3]
    # ---- or by channel standard channel names, such as mem.servos.throttle or mem.servos.rudder
    # mem.write_controller_1() will apply the modified PWM values to the controller's output vector.
    # ---- these outputs do not go directly to the servo-rail. they are instead handled by the RCIN_SERVO thread.

    while True:
        t0 = time.time()

        # read latest data
        [X, Y, Z, VT, ALPHA, BETA, PHI, THETA, PSI, P, Q, R] = mem.read_xh()
        mem.read_rcin()
        mem.read_servos()

        # \/ \/ \/ \/ \/ \/ \/ \/ \/  CONTROLLER CODE STARTS HERE  \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ #

        # manual passthrough
        mem.servos.throttle = mem.rcin.throttle
        mem.servos.flaps = mem.rcin.flaps

        # adjusting channels
        mem.servos.aileron += 4
        mem.servos[8] -= 4

        # /\ /\ /\ /\ /\ /\ /\ /\ /\  CONTROLLER CODE STOPS HERE  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ #

        # update controller data
        mem.write_controller_1()

        # sleep to maintain frequency
        sleep_time = max_sleep - (time.time() - t0)
        time.sleep(max(sleep_time, 0))
