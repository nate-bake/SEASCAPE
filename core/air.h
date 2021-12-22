#pragma once

#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <chrono>
#include <stdlib.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "Navio2/C++/Ublox.h"
#include "Navio2/C++/LSM9DS1.h"
#include "Navio2/C++/MPU9250.h"
#include "Navio2/C++/MS5611.h"
#include "Navio2/C++/ADC_Navio2.h"
#include "Navio2/C++/RCInput_Navio2.h"
#include "Navio2/C++/RCOutput_Navio2.h"

#include "mavlink/common/mavlink.h"
#include "serial_port.h"

#include <jsoncpp/json/value.h>
#include <jsoncpp/json/json.h>
#include <fstream>
#include <algorithm>
#include <iterator>
#include <map>

#define GETEKYDIR ("core/air.h")
#define PROJECTID (2333)
#define SHMSIZE (4096)

#include "air_config.h"