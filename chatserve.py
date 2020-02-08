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
    s.bind( ('', port) )

    # put socket on listen
    s.listen(5)

    while True:
        connection, addr = s.accept()
        print("Connected to", addr)
        msg = ""
        count = 0
        size = 0
        while True:
            data = connection.recv(CHARMAX)
            msg += str(data.decode())
            print("Client loop> " + msg)

            if count == 0:
                size = int(msg.partition(" ")[0])
                count += 1

            if size == len(msg):
                break
        
        msg = msg.partition(" ")[2]
        print("Client> " + msg)
        buffer = input("You> ")
        size = round(math.log(len(buffer),10))+2
        buffer = str(size + len(buffer)) + " " + buffer
        connection.sendall(buffer.encode())
        connection.close()

main()