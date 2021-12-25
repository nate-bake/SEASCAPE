# PEAS

python-based educational autopilot system

## DONE

- merged test.cpp into air.cpp, so we can include our own estimator and controller.
- replaced individual vectors with one large vector in shared memory that can be accessed by both C++ and Python.
- started a config.json file to define the indices for the shared memory.
- created Makefile for compiling.
- added MPU9250 to imu loop, allowing data from both IMUs to be available in the y vector.
- added some customization options to config file, including as ability to set loop rates.
- created python estimator/controller skeletons and connected them to air.cpp using a launch script.
- moved all sensitive code into 'core/' folder, added config validator, limited write capabilities on python side.
- implemented xh/controller vector choices in config, and tried to simplify reads on python side.
- added IMU calibration struct and logic to air.cpp, config.json, and launch.py.
- added logger thread and related config settings.

## TODO

- IMU calibration
  - add calibration scripts, save results as binary file.
    - flatten matrix and vector into a list of doubles.
    - once i know size of matrix i need to update load_calibration_file() in air.cpp.
  - then we need to finish implementing IMU update correction in InertialSensor.h.
  - once we finish calibration i should remove .bin files and add '/data/calibration' folder to .gitignore.
- look into adding i2c sensors.
- add our own estimator and controller to air.cpp.
- maybe break up air.cpp into multiple files to allow more flexibility.
- test servo loop cuz idk if the rcin and pwm scales are the same.
- update telemetry thread and maybe add some config settings for it.
- try to anticipate potential issues and reduce the probability that air.cpp process will ever crash.
- test like every individual piece in different config scenarios.

## SETUP / DEPENDENCIES

- install JSONCPP on Navio so that C++ can read config file. `sudo apt-get install libjsoncpp-dev`
- install Python IPC package on Navio so Python can access shared memory. `sudo pip3 install sysv_ipc`
- note that some of the Navio2 C++ libraries have been modified, hence the Emlid submodule is not included.
- be sure to actually clone the mavlink submodule thingy. `git submodule update --init`

## COMPILE / RUN

- `sudo python3 launch.py` should hopefully take care of everything.
