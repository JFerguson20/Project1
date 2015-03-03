Project1: Main.o Commands.o Disk.o
	g++ -o Project1 Main.o Commands.o Disk.o

Main.o: Main.cpp Main.h
	g++ -c Main.cpp

Commands.o: Commands.cpp Commands.h
	g++ -c Commands.cpp

Disk.o: Disk.cpp Disk.h
	g++ -c Disk.cpp

clean:
	rm Project1 Main.o Commands.o Disk.o
