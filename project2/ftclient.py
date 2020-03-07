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


CHARMAX = 514

def recvMsg(connection):
    msg = ""
    count = 0
    size = 0
    
    # Receive Loop
    while True:
        # block until data is received and decode to string
        data = connection.recv(CHARMAX)
        msg += str(data.decode())
        # print("msg:", msg)

        if count == 0:
            # read msg length from (<msg len>, msg)
            size = int(msg.partition(" ")[0])
            count += 1

        if size == len(msg):
            break
    
    # read msg from (<msg len>, msg)
    msg = msg.partition(" ")[2]
    return msg

#*******************************************
# Function Main
# Description: Creates socket server to listen for respective client connections
# Prerequisites: args[] contain: 
#   {chatserve.py <port>}
#*******************************************
def main():
    # Validate args
    if len(sys.argv) != 2:
        print("Invalid argument count")
        return
    
    # Create Socket and bind to port from args
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    port = int(sys.argv[1])
    print(port)
    s.connect(('flip3.engr.oregonstate.edu', port))
    # put socket on listen

    #****************
	# Server Loop
	#****************

    # Prompt user for response
    buffer = input("Client> ")


    # Get <size buffer> and <len(size buffer + ' ')>
    sizeB = len(buffer)
    size = len(str(sizeB)) + sizeB + 1

    # If (size+offset) length > (size), add 1 
    if len(str(sizeB)) < len(str(size)):
        size += 1
    buffer = str(size) + " " + buffer

    # Send message to client
    s.sendall(buffer.encode())

    # Receive and output message 
    msg = recvMsg(s)
    print(msg)

    s.close()

main()