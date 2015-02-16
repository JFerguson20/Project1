#student.make
# example working make file for use of the class Student
# files: source files:  student.cpp studentmain.cpp
#        header file:  student.h
# executable file: student
#

# first define target file : student 
#              dependencies are the object files that build the program

Project1: Main.o Commands.o
	g++ -o Project1 Main.o Commands.o

# now define how each object file is a target and list dependencies and how
#  to build that object file if any dependencies change

Main.o: Main.cpp Main.h
	g++ -c Main.cpp

Commands.o: Commands.cpp Commands.h
	g++ -c Commands.cpp

clean:
	rm Project1 Main.o Commands.o
