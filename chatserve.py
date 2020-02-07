import socket

def main():
    s = socket.socket()
    port = 12345

    # bind socket to port
    s.bind( ('', port) )

    # put socket on listen
    s.listen(5)

    while True:
        c, addr = s.accept()
        print("Got Connection from:", addr)

        c.send("From chatserver!")
        c.close()

main()