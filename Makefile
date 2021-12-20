default: clean_upper air

air: air.o Navio2/C++/Navio.o serial_port.o air.cpp air.h
	g++ air.o Navio2/C++/Navio.o serial_port.o -o air -std=c++11 -pthread -Wno-psabi -ljsoncpp

air.o: air.h Navio2/C++/Navio.o serial_port.o air.cpp
	g++ -c air.cpp -std=c++11 -Wno-psabi -pthread -Wno-psabi -ljsoncpp

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
