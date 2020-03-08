#******************************************
# Author: Zach Reed
# Description: Project 1 Chatserve
# Date: 2/9/2020
# References:
# 1.) Author: Python Software Foundation
#     Title:  The Python Standard Library - Sockets
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

class Setup:
    serverHost = None
    serverPort = None
    dataPort = None
    fileName = None


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


def formatBuffer(buffer):
    # Get <size buffer> and <len(size buffer + ' ')>
    sizeB = len(buffer)
    size = len(str(sizeB)) + sizeB + 1

    # If (size+offset) length > (size), add 1 
    if len(str(sizeB)) < len(str(size)):
        size += 1
    buffer = str(size) + " " + buffer
    return buffer

def formatListReq(args):
    command = "-l"
    clientHost = " " + socket.gethostname()
    clientHost = clientHost.replace(OSU, '')
    dataPort = " " + args[4]

    return command + clientHost + dataPort

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

    # print(command)
    setup.serverHost = sys.argv[1]
    setup.serverPort = int(sys.argv[2])

    # Create Socket and bind to port from args
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    s.connect((setup.serverHost+OSU, setup.serverPort))
    # put socket on listen

    # Prompt user for response
    msg = formatBuffer(buffer)

    #****************
	# Server Loop
	#****************
    socketRecv = socket.socket()
    socketRecv.bind(('flip3'+OSU, setup.dataPort))
    socketRecv.listen(5)
    
    
    s.sendall(msg.encode())
    s.close()
    print("After sendall")
    connection, addr = socketRecv.accept()


    print("Connected to:"+str(addr))


    if command == LS:
        print("Receiving directory structure from " + setup.serverHost + ":" + str(setup.serverPort))
        msg = recvDir(connection)
        print(msg)


    elif command == GET:
        msg = recvFile(connection, setup)
        print(msg)
    
    socketRecv.close()
 
main()