# ** Zach Reed
# ** CS-372
# ** Makefile for chatclient.c 
# *********************************************

CXX = gcc	#enables gcc as compiler

#target: dependencies
#	action

# Executable output, dependencies, and action ***************************
all: chatclient

chatclient: chatclient.o
	$(CXX) chatclient.o -o chatclient

#tells compiler to create object without executable when modified
chatclient.o: chatclient.c
	$(CXX) -c -g chatclient.c


#Removes object files and output ****************************************
clean:
	rm *.o chatclient
