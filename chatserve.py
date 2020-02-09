import socket
import sys
import math
import string

CHARMAX = 504

def main():

    if len(sys.argv) != 2:
        print("Invalid argument count")
        return

    s = socket.socket()
    port = int(sys.argv[1])

    # bind socket to port
    s.bind(('', port))

    # put socket on listen
    s.listen(5)

    quit = False
    #****************
	# Server Loop
	#****************
    while True:
        connection, addr = s.accept()
        print("New Session connected to:"+str(addr))
        recv = 0
        client = ""

        #****************
	    # Chat loop
	     #****************
        while not quit:
            msg = ""
            count = 0
            size = 0
            
            # Receive Loop
            while True:
                # block until data is received and decode to string
                data = connection.recv(CHARMAX)
                msg += str(data.decode())

                if count == 0:
                    # read msg length from (<msg len>, msg)
                    size = int(msg.partition(" ")[0])
                    count += 1

                if size == len(msg):
                    break
            
            # read msg from (<msg len>, msg)
            msg = msg.partition(" ")[2]
            recv += 1

            # If new session, set client handle
            if recv == 1:
                client = msg
                continue
            
            # If client quit, end chat loop
            if msg == "\quit":
                break

            print(client+"> " + msg)

            # Prompt user for response
            buffer = input("Server> ")

            # if input is to quit connection
            if buffer == "\quit":
                quit = True

            # Get <size buffer> and <len(size buffer + ' ')>
            sizeB = len(buffer)
            size = len(str(sizeB)) + sizeB + 1

		    # If (size+offset) length > (size), add 1 
            if len(str(sizeB)) < len(str(size)):
                size += 1
            buffer = str(size) + " " + buffer

            # Send message to client
            connection.sendall(buffer.encode())

        connection.close()
    s.close()
main()