#******************************************
# Author: Zach Reed
# Description: Project 2 ftclient
# Course: CS372-400
# Date: 3/8/2020
# References:
# 1.) Author: Python Software Foundation
#     Title:  The Python Standard Library - Sockets
# 2.) Author: Zach Reed
#	  Title:  Project 1 source code, CS344 Fall 2020
# 3.) Author: Brian "Beej Jorgensen" Hall
#	  Title:  Beej's Guide to Network Programming
#******************************************

import socket
import sys
import math
import string
from io import open

CHARMAX = 10000000000
LS = "-l "
GET = "-g "
ERROR = "-e "
OSU = ".engr.oregonstate.edu"

# holds setup variables from main args
class Setup:
    serverHost = None
    serverPort = None
    dataPort = None
    fileName = None

#*******************************************
# Function Receive Directory
# Description: adds <buffer size> to buffer
#*******************************************
def recvDir(socket):
    msg = ""
    count = 0
    size = 0
    sbytes = 0

    # Receive Loop
    while True:
        # block until data is received and decode to string
        data = socket.recv(CHARMAX)
        msg += str(data.decode())

        if count == 0:
            # read msg length from (<msg len>, msg)
            size = int(msg.partition(" ")[0])
            msg = msg.partition(" ")[2]

            if not msg.startswith(ERROR, 0, len(ERROR)):
                msg = msg.partition(LS)[2]

            else:
                msg = msg.partition(ERROR)[2]
                return msg

        sbytes = sbytes + len(data)

        if sbytes >= size:
            break

        count += 1

    
    # read msg from (<msg len>, msg)
    return msg

#*******************************************
# Function Receive File
# Description: gets bytes from recv() and writes to file if no error is presented
#*******************************************
def recvFile(socket, setup):
    msg = ""
    count = 0
    size = 0
    data = b""

    # Receive Loop
    while True:
        # block until data is received
        data = data + socket.recv(CHARMAX)
        if count == 0:
            # read msg length from (<msg len>, msg)
            msg = data.decode("utf8", errors="ignore")
            size = int(msg.partition(" ")[0])
            msg = msg.partition(" ")[2]
            offset = len(str(size)) + 1
            # move data to offset size string
            data = data[offset:]

            # if receiving file
            if not msg.startswith(ERROR, 0, len(ERROR)):
                print("Receiving \"" + setup.fileName + "\" from " + setup.serverHost +":"+str(setup.serverPort))
                # open file for writing
                file = open(setup.fileName, "wb")

            # if error from receiving file
            else:
                msg = msg.partition(ERROR)[2]
                return setup.serverHost +":"+str(setup.serverPort) + " says " + msg 

        # if transfer has ended, write total data array
        if len(data) + offset >= size:
            file.write(data)
            msg = "File transfer complete"
            break 

        count += 1

    return msg

#*******************************************
# Function Format Buffer
# Description: adds <buffer size> to buffer
#*******************************************
def formatBuffer(buffer):
    # Get <size buffer> and <len(size buffer + ' ')>
    sizeB = len(buffer)
    size = len(str(sizeB)) + sizeB + 1

    # If (size+offset) length > (size), add 1 
    if len(str(sizeB)) < len(str(size)):
        size += 1
    buffer = str(size) + " " + buffer
    return buffer

#*******************************************
# Function Format List Request
# Description: formats list string to send to fts
#*******************************************
def formatListReq(args):
    command = "-l"
    clientHost = " " + socket.gethostname()
    clientHost = clientHost.replace(OSU, '')
    dataPort = " " + args[4]

    return command + clientHost + dataPort

#*******************************************
# Function Format Get Request
# Description: formats get string to send to ftserver
#*******************************************
def formatGetReq(args):
    command = "-g"
    fileName = " " + args[4]
    dataPort = " " + args[5]
    clientHost = " " + socket.gethostname()
    clientHost = clientHost.replace(OSU, '')
    return command + fileName + dataPort + clientHost


#*******************************************
# Function Main
# Description: Connects to server and opens data port to receive directory or file information on.
# Prerequisites: args[] contain: 
#   {ftclient.py flip<1-3> <command> <optional file> <data port>
#*******************************************
def main():
    # Validate args
    buffer = None
    command = None


    setup = Setup()

    # if -l, check for valid arguments
    if len(sys.argv) == 5 and sys.argv[3] == "-l":
        command = LS
        buffer = formatListReq(sys.argv)
        setup.dataPort = int(sys.argv[4])

    # if -l, check for valid arguments
    elif len(sys.argv) == 6 and sys.argv[3] == "-g":
        command = GET
        buffer = formatGetReq(sys.argv)
        setup.dataPort = int(sys.argv[5])
        setup.fileName = sys.argv[4]

    else:
        print("Invalid Command")
        return

    # Setup server connection
    setup.serverHost = sys.argv[1]
    setup.serverPort = int(sys.argv[2])

    # Create Socket and bind to port from args
    socketSend = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    socketSend.connect((setup.serverHost+OSU, setup.serverPort))
    # put socket on listen

    # Prompt user for response
    msg = formatBuffer(buffer)

    # Open data port socket and listen on data port
    socketRecv = socket.socket()
    socketRecv.bind((socket.gethostname(), setup.dataPort))
    socketRecv.listen(5)
    
    # Send and close send-socket
    socketSend.sendall(msg.encode())
    socketSend.close()
    connection, _ = socketRecv.accept()

    # if command is List, recvDir()
    if command == LS:
        print("Receiving directory structure from " + setup.serverHost + ":" + str(setup.serverPort))
        msg = recvDir(connection)
        print(msg)

    # if command is Get, recvFile()
    elif command == GET:
        msg = recvFile(connection, setup)
        print(msg)
    
    # close receive-socket
    socketRecv.close()
 
main()