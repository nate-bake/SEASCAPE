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
- Moved sensitive code into `core/` folder, added config validator.
- Restricted the write capabilities on Python side.
- Implemented xh/controller vector choices in config, and tried to simplify reads on Python side.
- Added IMU calibration struct and logic to `air.cpp`, `config.json`, and `launch.py`.
- Added logger thread and related config settings.
- Implmented IMU adjustment to `InertialSensor.h` to apply calibration profile.
- Added IMU adjustment to built-in estimator in case the user decides not to apply calibration to y vector.
- Improved IMU-related prelaunch checks.
- Added dependency installation to launch routine.
- Extracted estimator_0 and controller_0 into dedicated files.
- Added type validation to config checker.
- Added names to some servo channels to make controller development easier.
- Enhanced memory helpers for Python estimator and controller.
- Fixed servo loop bug and added prelaunch check for mode PWM ranges.
- Enforced manual RC mode as default.
- Enabled logger to save incrementally in the event of a crash/shutdown.


## TODO

- IMU calibration
  - Add calibration scripts, save results as binary file.
    - Flatten vector and matrix into a list of doubles (row-major).
  - Once we finish calibration I should remove `.bin` files and add tell git to ignore them.
- Look into adding I2C sensors.
- Add our own estimator and controller to `air.cpp`.
- Maybe break up `air.cpp` into multiple files.
- Update telemetry thread and maybe add some config settings for it.
- Try to anticipate potential issues and reduce the probability that `air.cpp` process will ever crash.
- Test like every individual piece in different config scenarios.
  - Especially servo thread since I don't know if RCIN and PWM scales are the same.
- Documentation overview and config setting explanations.
- Maybe remove vector keys from config.json file.

## INSTALLATION / EXECUTION

- `cd PEAS/`
- `sudo python3 launch.py` should hopefully take care of everything.
  - The `libjsoncpp-dev` apt package will be installed.
  - The `jsonschema` pip module will be installed.
  - The `sysv_ipc` pip module will be installed.
  - The mavlink submodule will be cloned if not already.
- Note that some existing Navio2 libraries have been modified. Hence the Emlid submodule is not included.
