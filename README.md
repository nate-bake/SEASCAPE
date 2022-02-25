<h1><b>SEASCAPE</b> -- SIMPLE EDUCATIONAL AUTOPILOT SYSTEM with C++ ARCHITECTURE & PYTHON EXTERIOR</h1>

This repository contains a user-friendly solution for testing new autopilot algorithms on a RaspberryPi.
It is designed to utilize the sensors present on the <a href="https://navio2.emlid.com/">Navio2 Autopilot HAT</a>, and was built around the <a href="https://docs.emlid.com/navio2/dev/navio-repository-cloning/">drivers and examples provided by Emlid.</a><br>
This system handles sensor interaction and provides data from GPS, barometer, and both onboard IMUs. It also contains built-in implementations of estimation and control algorithms, providing a basis for comparison and facilitating flight tests. These core functions are implemented in a multithreaded C++ process to optimize performance, and additional Python processes are provided for a implementing custom estimator and controller. The multithreaded structure adds modularity to the system which allows users to adjust the configuration via JSON properties.

<br>

## INSTALLATION / EXECUTION

After cloning, the launch file should hopefully take care of everything:

```
git clone https://github.com/nate-bake/SEASCAPE.git
cd SEASCAPE/
sudo python3 launch.py
```

- The `libjsoncpp-dev` apt package will be installed.
- The `jsonschema` pip module will be installed.
- The `sysv_ipc` pip module will be installed.
- The mavlink submodule will be cloned if not already.
- Note that some existing Navio2 libraries have been modified. Hence the Emlid submodule is not included.

<br>

To enable the program to launch on boot:

```
cd SEASCAPE/
sudo mv SEASCAPE.service /lib/systemd/system/SEASCAPE.service
sudo systemctl daemon-reload
sudo systemctl enable SEASCAPE.service
sudo reboot
```

- First you may need to modify the paths in SEASCAPE.service.
- Use `top` or `sudo systemctl status SEASCAPE.service` to see if the processes are running.
- Use `sudo killall air` to terminate the program.

<br>

## THREAD VISUALIZATION

![](https://user-images.githubusercontent.com/34242063/151286155-4905c772-6fcf-41a0-b203-5f103e4db80e.gif)

- ![#F9B3A7](https://via.placeholder.com/15/F9B3A7/000000?text=+) &nbsp;Core Threads _[C++]_
- ![#F14124](https://via.placeholder.com/15/F14124/000000?text=+) &nbsp;Custom Threads _[Python]_
- ![#5ECCF3](https://via.placeholder.com/15/5ECCF3/000000?text=+) &nbsp;Shared Vectors
- ![#BFBFBF](https://via.placeholder.com/15/BFBFBF/000000?text=+) &nbsp;I/O

<br>

- As seen above, individual threads can be toggled or adjusted via `config.json`.
  - For example, one can elect to use the built-in controller in conjunction with the custom estimator.
- Each value within the shared vectors is modified by exactly one thread, mitigating risk of race conditions.
- One additional 'LOGGER' thread is not depicted. It can access all vectors and produce a CSV file.

<br>

## THREAD DETAILS

### IMU_ADC

- Reads the following data and stores it in the `y` vector.
  - Accelerometer, gyroscope, and magnetometer data from both onboard IMUs [*LSM9DS1, MPU9250*].
    - Includes both raw and calibrated data from each IMU.
  - Voltage from board, servo rail, and power connector.
- Config settings:
  - `"ENABLED": true`
  - `"RATE": 100`
    - Loop frequency defined in hertz [must be > 0].
  - Sensor switches (make sure at least one IMU is enabled):
    - `"USE_LSM9DS1": true`
    - `"USE_MPU9250": true`
    - `"USE_ADC": true`
  - `"PRIMARY_IMU": "MPU9250"`
    - If both onboard IMUs are enabled, this will dictate which is considered _IMU_1_.

### GPS_BARO

- Reads the following data and stores it in the `y` vector.
  - GPS position, velocity, and fix status.
  - Pressure reading from onboard barometer [*MS5611*].
- Config settings:
  - `"ENABLED": true`
  - `"RATE": 10`
    - Loop frequency defined in hertz. Note that any rate above 5 or above 20 is likely unattainable for the GPS or barometer, respectively.
  - `"USE_GPS": true`
  - `"USE_MS5611": true`

### RCIN_SERVO

- Reads RCInput values for each channel and stores it in the `rcin` vector.
- Determines flight mode from specified RCInput channel.
  - If flight mode is _MANUAL_, update servo-rail PWM from RCInput values.
    - _MANUAL_ mode is the default mode.
  - If flight mode is _AUTO_, update servo-rail PWM from specified controller vector.
  - If flight mode is _SEMI-AUTO_, behave like auto mode unless RCInput joysticks are not centered.
- Config settings:
  - `"ENABLED": true`
  - `"RATE": 50`
    - Loop frequency defined in hertz [recommended: = _PWM_FREQUENCY_].
  - `"PWM_FREQUENCY": 50`
    - Rate at which PWM pulses are sent.
  - `"CONTROLLER_VECTOR_TO_USE": 0`
    - Controller to reference when in _AUTO_ mode. Either 0 or 1.
    - If the specified controller is not enabled, only MANUAL mode will be available.
  - The following channels are needed for logging and built-in controller purposes. Valid range: [1,14].
    - `"THROTTLE_CHANNEL": 1`
    - `"AILERON_CHANNEL": 2`
    - `"ELEVATOR_CHANNEL": 3`
    - `"RUDDER_CHANNEL": 4`
    - `"FLAPS_CHANNEL": 6`
    - `"FLIGHT_MODES/MODE_CHANNEL": 5`
      - RC channel which is used to determine flight mode.
  - Flight mode ranges defined below are lower-limit inclusive, upper-limit exclusive:
    - `"FLIGHT_MODES/MANUAL_RANGE/LOW": 750`
    - `"FLIGHT_MODES/MANUAL_RANGE/HIGH": 1250`
    - `"FLIGHT_MODES/SEMI-AUTO_RANGE/LOW": 1250`
    - `"FLIGHT_MODES/SEMI-AUTO_RANGE/HIGH": 1750`
    - `"FLIGHT_MODES/AUTO_RANGE/LOW": 1750,`
    - `"FLIGHT_MODES/AUTO_RANGE/HIGH": 2250`
  - `"FLIGHT_MODES/SEMI-AUTO_DEADZONE": 50`
    - Amount required to move either aileron or elevator joystick away from 1500 to take manual control.
  - The following are used to clip RCInput and controller channel values before sending to servo-rail.
    - `"MIN_THROTTLE": 1000`
    - `"MAX_THROTTLE": 1750`
    - `"MIN_SERVO": 1250`
    - `"MAX_SERVO": 1750`

### ESTIMATOR_0

- Built-in estimator implemented in C++.
  - TODO include more info here once we actually implement it.
- Reads sensor data from the `y` vector and updates values in the `xh_0` vector.
- Requires GPS and one IMU to be enabled in order to function properly. If both onboard IMUs are enabled, _IMU_1_ will be used (defined by `PRIMARY_IMU` in `config.json`).
- Config settings:
  - `"ENABLED": true`
  - `"RATE": 100`
    - Loop frequency defined in hertz [must be > 0].

### ESTIMATOR_1

- Custom estimator to be implemented in Python.
- Reads sensor data from the `y` vector and updates values in the `xh_1` vector.
  - Read/write actions to shared memory have been abstracted and are handled by `core/shared_mem_helper_estimator_1.py`.
  - Can elect to use calibrated IMU data, or raw if you prefer to do you own calibration routine.
- Config settings:
  - `"ENABLED": true`
  - `"RATE": 100`
    - Loop frequency defined in hertz [must be > 0].

### CONTROLLER_0

- Built-in controller implemented in C++.
- Reads state data from the specified xh vector and updates values in the `controller_0` vector to be applied to servos.
- Config settings:
  - `"ENABLED": true`
  - `"RATE": 50`
    - Loop frequency defined in hertz [must be > 0].
  - `"XH_VECTOR_TO_USE": 0`
    - `0` for built-in estimator, `1` for custom estimator.
    - Make sure that the estimator thread is enabled for the selected vector.

### CONTROLLER_1

- Custom controller implemented in Python.
- Reads state data from the specified xh vector and updates values in the `controller_1` vector to be applied to servos.
  - Read/write actions to shared memory have been abstracted and are handled by `core/shared_mem_helper_controller_1.py`.
    - Channels in custom servo object can be accessed via:
      - Array indexing (1-based) : `mem.servos[2]`
      - Object attributes (channel-names defined in `config.json`): `mem.servos.aileron`
    - This servo object does not modify PWM directly, but rather proposes new vales to the _RCIN_SERVO_ thread.
- Config settings:
  - `"ENABLED": true`
  - `"RATE": 50`
    - Loop frequency defined in hertz [must be > 0].
  - `"XH_VECTOR_TO_USE": 1`
    - `0` for built-in estimator, `1` for custom estimator.
    - Make sure that the estimator thread is enabled for the selected vector.

### LOGGER

- Creates a CSV file and appends data from the selected vectors at a specified rate.
- Logs can be found in `data/logs/` and are named based on time of launch.
- Config settings:
  - `"ENABLED": true`
  - `"RATE": 50`
    - Loop frequency defined in hertz [must be > 0].
  - Vectors to log [if associated threads are disabled, those columns are automatically omitted]:
    - `"LOG_SENSOR_DATA": true`
    - `"LOG_ESTIMATOR_0": true`
    - `"LOG_ESTIMATOR_1": false`
    - `"LOG_CONTROLLER_0": false`
    - `"LOG_CONTROLLER_1": true`
    - `"LOG_RCIN_SERVO": true`

<br>

## VECTOR DETAILS

- The keys for each vector are defined in [core/keys.json](core/keys.json) but should not be modified unless you really know what you're doing.

<br>

## TODO

- IMU calibration
  - Add calibration scripts, save results as binary file.
    - Can do it in Python rather than C++.
    - Flatten vector and matrix into a list of doubles (row-major).
  - Should it be part of pre-launch process, or stand-alone?
  - Once we finish calibration I should remove `.bin` files and add tell git to ignore them.
- Telemetry
  - Should we even include it?
  - What data to send?
    - GPS data?
    - Things from xh?
    - Battery info? [*If so IDK how to interpret the voltages*]
    - Flight mode?
- Look into adding I2C sensors.
- Add functionality to estimator_0.cpp and controller_0.cpp
- Documentation overview and config setting explanations.
- Test like every individual piece in different config scenarios.
  - Especially servo thread since IDK if I set up the channel indexing and clipping correctly.
  - Try to anticipate potential issues and reduce the probability that `air.cpp` process will ever crash.
- Waypoint tracking??
