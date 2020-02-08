import socket
import sys

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
        while True:
            data = connection.recv(CHARMAX)
            msg += str(data.decode())
            print("From client", msg)

            if "\n" in msg:
                break
        print("From client", msg)
        connection.sendall(msg)
        connection.close()

main()