default: clean_upper air

air: air.o imu_adc.o gps_baro.o rcin_servo.o telemetry.o logger.o estimator_0.o controller_0.o air_config.o Navio
	g++ *.o Navio2/Navio.o -o air -std=c++11 -Wno-psabi -pthread -ljsoncpp

air.o:
	g++ -c air.cpp -std=c++11 -Wno-psabi -pthread

imu_adc.o: 
	g++ -c imu_adc.cpp -std=c++11 -Wno-psabi

gps_baro.o: 
	g++ -c gps_baro.cpp -std=c++11 -Wno-psabi

rcin_servo.o: 
	g++ -c rcin_servo.cpp -std=c++11 -Wno-psabi

telemetry.o: 
	g++ -c telemetry.cpp -std=c++11 -Wno-psabi

logger.o:
	g++ -c logger.cpp -std=c++11 -Wno-psabi

estimator_0.o:
	g++ -c estimator_0.cpp -std=c++11 -Wno-psabi

controller_0.o:
	g++ -c controller_0.cpp -std=c++11 -Wno-psabi

air_config.o:
	g++ -c air_config.cpp -std=c++11 -Wno-psabi -ljsoncpp

Navio:
	$(MAKE) -C Navio2/

clean_upper:
	-rm -f air
	-rm -f *.o

clean:
	-rm -f air
	-rm -f *.o
	-rm -f Navio2/*.o
	-rm -f Navio2/C++/*.o
