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
    while True:
        connection, addr = s.accept()
        print("Connected to", addr)
        recv = 0

        while not quit:
            msg = ""
            count = 0
            size = 0
            
            # Receive message from client
            while True:
                data = connection.recv(CHARMAX)
                msg += str(data.decode())
                # print("Client loop> " + msg)

                if count == 0:
                    size = int(msg.partition(" ")[0])
                    count += 1

                if size == len(msg):
                    break
            
            msg = msg.partition(" ")[2]
            recv += 1

            if recv == 1:
                #set to handler for client
                recv = recv

            if msg == "\quit":
                break

            print("Client> " + msg)

            # Prompt user for response
            buffer = input("You> ")

            # if input is to quit connection
            if buffer == "\quit":
                quit = True

            size = math.floor(math.log(len(buffer),10))+2
            buffer = str(size + len(buffer)) + " " + buffer

            # Send message to client
            connection.sendall(buffer.encode())

        connection.close()

main()