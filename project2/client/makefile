# ** Zach Reed
# ** CS-372
# ** Makefile for ftserver.c 
# *********************************************

CXX = gcc	#enables gcc as compiler

#target: dependencies
#	action

# Executable output, dependencies, and action ***************************
all: ftserver

ftserver: ftserver.o
	$(CXX) ftserver.o -o ftserver

#tells compiler to create object without executable when modified
ftserver.o: ftserver.c
	$(CXX) -c -g ftserver.c


#Removes object files and output ****************************************
clean:
	rm *.o ftserver
