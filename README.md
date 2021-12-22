# PEAS

python-based educational autopilot system

## DONE

- merged test.cpp into air.cpp, so we can include our own estimator and controller.
- replaced individual vectors with one large vector in shared memory that can be accessed by both C++ and Python.
- started a config.json file to define the indices for the shared memory.
- created Makefile for compiling.
- added MPU9250 to imu loop, allowing data from both IMUs to be available in the y vector.
- added option to set loop rates in config file.
- added some customization options to config file. not sure if this is the best solution though.
- created python estimator/controller skeletons and connected them to air.cpp using a launch script.
- moved all sensitive code into 'core/' folder, added config validator, limited write capabilities on python side.

## TODO

- add IMU calibration as a separate routine and configure air.cpp to use the profile and recommend new calibration if outdated.
- add our own estimator and controller to air.cpp and figure out how we want to log results.
- maybe break up air.cpp into multiple files to allow more flexibility
- test servo loop cuz idk if the rcin and pwm scales are the same.
- should telemetry be on the c++ or python side?
- actually implement the xh/controller vector choices in config, and maybe find a way to simplify reads on python side.

## SETUP / DEPENDENCIES

- install JSONCPP on Navio so that C++ can read config file. `sudo apt-get install libjsoncpp-dev`
- install Python IPC package on Navio so Python can access shared memory. `sudo pip3 install sysv_ipc`
- i modified some of the files in the Navio2 folder to get the GPS to work properly, so we can no longer include it as a submodule from Emlid.

## COMPILE / RUN

- `sudo python3 launch.py` should hopefully take care of everything.
