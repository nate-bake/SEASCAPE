import time


def estimator_loop(mem, max_sleep):

    gps = imu = 0
    while True:
        initial_time = time.time()
        # =====ESTIMATOR CODE STARTS HERE==================================
        # Predict step here
        # Measurement correction step here (depends on which sensors available)
        if imu == mem.read("IMU_UPDATES"):
            time.sleep(0.001)
            continue
        imu = mem.read("IMU_UPDATES")
        print(mem.read("IMU_1_AX"))
        if gps != mem.read("GPS_UPDATES"):
            gps = mem.read("GPS_UPDATES")
            pass  # do estimation with gps here.
        else:
            pass  # do estimation without gps here.
        # write estimated values to the xh array.
        # ======ESTIMATOR CODE STOPS HERE===================================
        sleep_time = max_sleep - (time.time() - initial_time)
        time.sleep(max(sleep_time, 0))
