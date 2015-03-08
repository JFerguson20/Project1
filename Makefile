Project1: Main.o Commands.o Disk.o Remote.o Sh.o
	g++ -o Project1 Main.o Commands.o Disk.o Remote.o Sh.o

Main.o: Main.cpp Main.h
	g++ -c Main.cpp

Commands.o: Commands.cpp Commands.h
	g++ -c Commands.cpp

Disk.o: Disk.cpp Disk.h
	g++ -c Disk.cpp

Remote.o: Remote.cpp Remote.h
	g++ -c Remote.cpp

Sh.o: Sh.cpp Sh.h
	g++ -c Sh.cpp
clean:
	rm Project1 Main.o Commands.o Disk.o Remote.o Sh.o
