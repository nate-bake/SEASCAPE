# PEAS

Python-based Educational Autopilot System

## DONE

- Merged `test.cpp` into `air.cpp`, so we can include our own estimator and controller.
- Replaced individual vectors with one large vector in shared memory to be accessed by both C++ and Python.
- Started a `config.json` file to define the indices for the shared memory.
- Created Makefile for compiling.
- Added MPU9250 to IMU loop, allowing data from both IMUs to be available in the y vector.
- Added some customization options to config file, including as ability to set loop rates.
- Created Python estimator/controller skeletons and connected them to `air.cpp` using a launch script.
- Moved sensitive code into `core/` folder, added config validator, limited write capabilities on Python side.
- Implemented xh/controller vector choices in config, and tried to simplify reads on Python side.
- Added IMU calibration struct and logic to `air.cpp`, `config.json`, and `launch.py`.
- Added logger thread and related config settings.

## TODO

- IMU calibration
  - Add calibration scripts, save results as binary file.
    - Flatten matrix and vector into a list of doubles.
    - Once I know size of matrix I need to update `load_calibration_file()` in `air.cpp`.
  - Then we need to finish implementing IMU update correction in `InertialSensor.h`.
  - If someone wants to disable IMU calibration in y vector, we probably need to do it in our estimator.
  - Once we finish calibration I should remove `.bin` files and add tell git to ignore them.
- Look into adding I2C sensors.
- Add our own estimator and controller to `air.cpp`.
- Maybe break up `air.cpp` into multiple files to allow more flexibility.
- Update telemetry thread and maybe add some config settings for it.
- Try to anticipate potential issues and reduce the probability that `air.cpp` process will ever crash.
- Test like every individual piece in different config scenarios.
  - Especially servo thread since I don't know if RCIN and PWM scales are the same.

## SETUP / DEPENDENCIES

- Install JSONCPP on Navio so that C++ can read config file. `sudo apt-get install libjsoncpp-dev`
- Install Python IPC package on Navio so Python can access shared memory. `sudo pip3 install sysv_ipc`
- Note that some of the Navio2 C++ libraries have been modified, hence the Emlid submodule is not included.
- Be sure to actually clone the mavlink submodule thingy. `git submodule update --init`

## COMPILE / RUN

- `sudo python3 launch.py` should hopefully take care of everything.
