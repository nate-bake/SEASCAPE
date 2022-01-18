default: clean_upper air

air: air.o logger.o estimator_0.o controller_0.o air_config.o serial_port.o Navio2/C++/Navio.o
	g++ air.o logger.o estimator_0.o controller_0.o air_config.o serial_port.o Navio2/C++/Navio.o -o air -std=c++11 -pthread -Wno-psabi -ljsoncpp

air.o: air.h air.cpp
	g++ -c air.cpp -std=c++11 -Wno-psabi -pthread -Wno-psabi -ljsoncpp

logger.o: logger.cpp
	g++ -c logger.cpp -std=c++11 -Wno-psabi -pthread -Wno-psabi -ljsoncpp

estimator_0.o: estimator_0.cpp
	g++ -c estimator_0.cpp -std=c++11 -Wno-psabi -pthread -Wno-psabi -ljsoncpp

controller_0.o: controller_0.cpp
	g++ -c controller_0.cpp -std=c++11 -Wno-psabi -pthread -Wno-psabi -ljsoncpp

air_config.o: air_config.h air_config.cpp
	g++ -c air_config.cpp -std=c++11

serial_port.o: serial_port.h serial_port.cpp
	g++ -c serial_port.cpp -std=c++11

Navio2/C++/Navio.o:
	$(MAKE) -C Navio2/C++

clean_upper:
	-rm -f *.o
	-rm -f air

clean:
	-rm -f *.o
	-rm -f Navio2/C++/*.o
	-rm -f air
