Author: Zach Reed
Description: Project 2 Readme
Email: reedz@oregonstate.edu
Course: CS372-400
Date: 3/8/2020
References:
1.) Author: Zach Reed
    Title:  Project 1 source code, CS344 Fall 2020


The steps below outline how to setup chatclient and chatserve. 
Steps must be done in order to ensure connection between client-server.

1.  All testing was performed on flip. SSH to flip2.engr.oregonstate.edu and unzip the reedz_Project2.zip file.

2.  Compile ftserver.c using the makefile provided. 
    Be in the working directory of the unzipped files and type:

    $ make

3.  Type the following to start ftserver:

    $ ftserver <server port>

4.  Start ftclient.py with python3 and specifiy a port number.
    If python is an alias for python3, 'python' can be used for execution.
    Type the following to start the server:

    $ python3 ftclient.py flip<1-3> <server port> <command> <optional file> <data port>

