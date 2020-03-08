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


def recvMsg(connection):
    msg = ""
    count = 0
    size = 0
    
    # Receive Loop
    while True:
        # block until data is received and decode to string
        data = connection.recv(CHARMAX)
        msg += str(data.decode())
        print(msg)
        if count == 0:
            # read msg length from (<msg len>, msg)
            size = int(msg.partition(" ")[0])
            count += 1

        if size == len(msg):
            break
    
    # read msg from (<msg len>, msg)
    msg = msg.partition(" ")[2]
    return msg


def recvFile(connection, fileName):
    msg = ""
    count = 0
    size = 0
    sbytes = 0

    # Receive Loop
    while True:
        # block until data is received and decode to string
        data = connection.recv(CHARMAX)
        msg = data.decode("utf-8", errors='ignore')

        if count == 0:
            # read msg length from (<msg len>, msg)
            size = int(msg.partition(" ")[0])
            msg = msg.partition(" ")[2]

            if not msg.startswith(ERROR, 0, len(ERROR)):
                file = open(fileName, "w")

            else:
                print("Error from server")
                break

        count += 1
        file.write(msg)
        sbytes = sbytes + len(data)

        if sbytes >= size:
            break 

    # read msg from (<msg len>, msg)



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

    # Send message to client

    # Receive and output message 
    recvFile(s, conn.fileName)
    # msg = recvMsg(s)

    if msg.startswith(LS, 0, len(LS)):
        print("Receiving directory structure from " + conn.serverHost + ":" + str(conn.serverPort))
        msg = msg.partition(LS)[2]

    if msg.startswith(GET, 0, len(GET)):
        file = open(conn.fileName, "w+")
        file.write(msg.partition(GET)[2])
        msg = "Receiving file:" + conn.serverHost + ":" + str(conn.serverPort)
    
    
    print(msg)

    s.close()

main()