Author: Zach Reed
Email: reedz@oregonstate.edu
Date: 2/9/2020

The steps below outline how to setup chatclient and chatserve. 
Steps must be done in order to ensure connection between client-server.

1.  Compile chatclient.c with using the makefile provided. 
    Be in the working directory of the unzipped files and type:
    $ make 

2.  Start chatserve.py with python3 and specifiy a port number.
    If python is an alias for python3, 'python' can be used for execution.
    Type the following to start the server:
    $ python3 chatserve.py <port number>


3.  Start chatclient and connect to chatserve.
    To start the chatclient, the user will need to specify the machine host type 
    and the port number that chatserve is running on. Type the following:
    $ chatclient localhost <chatserve port>

4.  Begin execution of messages between client and server.
