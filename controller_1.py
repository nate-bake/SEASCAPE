import time


def controller_loop(mem, max_sleep):

    xh_updates = 0
    while True:
        initial_time = time.time()
        if xh_updates == mem.read("XH_1_UPDATES"):
            time.sleep(0.001)
            continue
        xh_updates = mem.read("XH_1_UPDATES")
        # ======CONTROLLER CODE STARTS HERE===============================================
        mem.write("CONTROLLER_1_0", mem.read("RCIN_0"))
        mem.write("CONTROLLER_1_1", mem.read("RCIN_1"))
        mem.write("CONTROLLER_1_2", mem.read("RCIN_2"))
        mem.write("CONTROLLER_1_3", mem.read("RCIN_3"))
        mem.write("CONTROLLER_1_4", mem.read("RCIN_4"))
        mem.write("CONTROLLER_1_5", mem.read("RCIN_5"))
        # I UNDERSTAND THAT THIS IS NOT VERY INTUITIVE
        # =======CONTROLLER CODE STOPS HERE ======================================
        sleep_time = max_sleep - (time.time() - initial_time)
        time.sleep(max(sleep_time, 0))
