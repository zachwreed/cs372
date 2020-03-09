# Author: Zach Reed
# Course: CS-372-400
# Description: Makefile for ftserver.c 
# Date: 3/8/2020

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
