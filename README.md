<h1>PYTHON-BASED EDUCATIONAL AUTOPILOT SYSTEM</h1>

<br>

## INSTALLATION / EXECUTION

This should hopefully take care of everything:
```
cd PEAS/
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
  cd PEAS/
  sudo mv PEAS.service /lib/systemd/system/PEAS.service
  sudo systemctl daemon-reload
  sudo systemctl enable PEAS.service
  sudo reboot
  ```
- First you may need to modify the paths in PEAS.service.
- Use `top` or `sudo systemctl status PEAS.service` to see if the processes are running.
- Use `sudo killall air` to terminate the program.

<br>

## THREAD VISUALIZATION

![](https://user-images.githubusercontent.com/34242063/149874441-a36b47b8-1806-4526-be47-73a8646add3b.png)

- ![#F9B3A7](https://via.placeholder.com/15/F9B3A7/000000?text=+) &nbsp;Core Threads
- ![#F14124](https://via.placeholder.com/15/F14124/000000?text=+) &nbsp;Custom Threads [*Python*]
- ![#5ECCF3](https://via.placeholder.com/15/5ECCF3/000000?text=+) &nbsp;Shared Vectors
- ![#BFBFBF](https://via.placeholder.com/15/BFBFBF/000000?text=+) &nbsp;I/O

<br>

- Individual threads can be toggled / adjusted by editing config.json.
  - For example, one can configure CONTROLLER_0 to read from *xh_1* rather than *xh_0*.
  - The RCIN_SERVO thread could also be configured to reference *controller_1* when in AUTO mode.
- Note that each shared vector is only modified by one thread, mitigating risk of race conditions.
- One additional 'LOGGER' thread is not depicted. It can access all vectors and produce a .csv file.

<br>

## TODO

- IMU calibration
  - Add calibration scripts, save results as binary file.
    - Flatten vector and matrix into a list of doubles (row-major).
  - Once we finish calibration I should remove `.bin` files and add tell git to ignore them.
- Look into adding I2C sensors.
- Add our own estimator and controller to core.
- Maybe break up `air.cpp` into multiple files.
- Update telemetry thread and maybe add some config settings for it.
  - Figure out what to do with ADC data.
- Try to anticipate potential issues and reduce the probability that `air.cpp` process will ever crash.
- Test like every individual piece in different config scenarios.
  - Especially servo thread since I don't know if RCIN and PWM scales are the same.
- Documentation overview and config setting explanations.

<br>

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
- Removed memory keys from config.json and hid them in core/keys.json.
- Added system service for launching. Not sure if this is really what we want though.
