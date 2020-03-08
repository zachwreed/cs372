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

CHARMAX = 100000000
LS = "-l "
GET = "-g "
ERROR = "-e "
OSU = ".engr.oregonstate.edu"

class Connection:
    serverHost = None
    serverPort = None
    dataPort = None
    fileName = None


def recvDir(connection):
    msg = ""
    count = 0
    size = 0
    sbytes = 0

    # Receive Loop
    while True:
        # block until data is received and decode to string
        data = connection.recv(CHARMAX)
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


def recvFile(socket, conn):
    msg = ""
    count = 0
    size = 0
    sbytes = 0

    # Receive Loop
    while True:
        # block until data is received and decode to string
        data = socket.recv(CHARMAX)
        msg = data.decode("utf-8", errors='ignore')

        if count == 0:
            # read msg length from (<msg len>, msg)
            size = int(msg.partition(" ")[0])
            msg = msg.partition(" ")[2]

            if not msg.startswith(ERROR, 0, len(ERROR)):
                print("Receiving \"" + conn.fileName + "\" from " + conn.serverHost +":"+str(conn.serverPort))
                file = open(conn.fileName, "w")

            else:
                msg = msg.partition(ERROR)[2]
                return conn.serverHost +":"+str(conn.serverPort) + " says " + msg 

        file.write(msg)
        sbytes = sbytes + len(data)

        # if transfer has ended
        if sbytes >= size:
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
# Description: Creates socket server to listen for respective client connections
# Prerequisites: args[] contain: 
#   {chatserve.py <port>}
#*******************************************
def main():
    # Validate args
    buffer = None
    command = None

    conn = Connection()

    if len(sys.argv) == 5 and sys.argv[3] == "-l":
        command = LS
        buffer = formatListReq(sys.argv)
        conn.dataPort = int(sys.argv[4])

    elif len(sys.argv) == 6 and sys.argv[3] == "-g":
        command = GET
        buffer = formatGetReq(sys.argv)
        conn.dataPort = int(sys.argv[5])
        conn.fileName = sys.argv[4]

    else:
        print("Invalid Command")
        return

    # print(command)
    conn.serverHost = sys.argv[1]
    conn.serverPort = int(sys.argv[2])

    # Create Socket and bind to port from args
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    s.connect((conn.serverHost+OSU, conn.serverPort))
    # put socket on listen

    # Prompt user for response
    msg = formatBuffer(buffer)
    s.sendall(msg.encode())

    #****************
	# Server Loop
	#****************

    if command == LS:
        print("Receiving directory structure from " + conn.serverHost + ":" + str(conn.serverPort))
        msg = recvDir(s)
        print(msg)


    elif command == GET:
        msg = recvFile(s, conn)
        print(msg)

 
    s.close()

main()