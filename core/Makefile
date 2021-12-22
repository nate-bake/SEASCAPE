default: clean_upper air

air: air.o Navio2/C++/Navio.o serial_port.o air.o air_config.o
	g++ air.o Navio2/C++/Navio.o serial_port.o air_config.o -o air -std=c++11 -pthread -Wno-psabi -ljsoncpp

air.o: air.h air.cpp
	g++ -c air.cpp -std=c++11 -Wno-psabi -pthread -Wno-psabi -ljsoncpp

air_config.o: air_config.h
	g++ -c air_config.cpp -std=c++11

serial_port.o: serial_port.h
	g++ -c serial_port.cpp -std=c++11

Navio2/C++/Navio.o:
	$(MAKE) -C Navio2/C++

clean_upper:
	-rm -f *.o
	-rm -f test

clean:
	-rm -f *.o
	-rm -f Navio2/C++/*.o
	-rm -f test
	-rm -f air
