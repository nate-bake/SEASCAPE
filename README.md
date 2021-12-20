# PEAS

python-based educational autopilot system

- DONE
	- 
	 - merged test.cpp into air.cpp, so we can include our own estimator and controller.
	 - replaced individual vectors with one large vector in shared memory that can be accessed by both C++ and Python.
	 - started a config.json file to define the indices for the shared memory.
	 - created Makefile for compiling.

- TODO
	- 
	 - add IMU calibration as a separate routine and configure air.cpp to reference the calibration profile and recommend a new calibration if out of date.
	 - add support for MPU and look into generalizing IMU reads.
	 - add our own estimator and controller to air.cpp and figure out how we want to log results.
	 - create a test.py skeleton and sync it with air.cpp using a launch script.
	 - figure out better ways to protect our code from goons.
	 - add more customization options to config file.
	 - maybe break up air.cpp into multiple files to allow more flexibility?

	
 - SETUP NOTES: 
	 - 
	 - need to install JSONCPP on Navio so that C++ can read config file. `sudo apt-get install libjsoncpp-dev`
	 - need to install Python IPC package on Navio so that Python side can access the shared memory vector.  `pip3 install sysv_ipc`
	 - i modified some of the files in the Navio2 folder to get the GPS to work properly, so we can no longer include it as a submodule from Emlid.
	 - TO COMPILE/RUN:
		 -  
		 - `git submodule update --init --recursive` to pull mavlink files if you didn't get them while cloning.
		 - `make clean`
		 - `make`
		 - `./air`
		 - `python3 shared_test.py` to test reading shared vector.
