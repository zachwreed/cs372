/***********************************************
** Author: Zach Reed
** Description: Project 2 ftserver
** Course: CS372-400
** Date: 3/8/2020
** References:
** 1.) 	Author: Benjamin Brewster
		Title:  Client Code for Module 4, CS344 Fall 2019
		Publisher: Oregon State
** 2.) 	Author: Zach Reed
		Title:  OTP Program 4 source code, CS344 Fall 2019
** 3.) 	Author: Zach Reed
		Title:  Project 1 source code, CS344 Fall 2020
** 4.)  Author: Brian "Beej Jorgensen" Hall
		Title:  Beej's Guide to Network Programming
************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h> 
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <math.h>
#include <errno.h>

#define MSGMAX 65020
#define INMAX 65000
#define LENMAX 10
#define LS "-l "
#define GET "-g "
#define ERROR "-e "
#define OSU ".engr.oregonstate.edu"


/***********************************************
** Function: Error Handler
** Description: Sends message to stderr and exits with status
************************************************/
void error(const char *msg) { 
	perror(msg); 
} 


/***********************************************
** Function: Error Handler
** Description: Sends message to stderr and exits with status
************************************************/
int getDir(char* buffer) {
	DIR *directory;
	struct dirent *dFile;
	char file[MSGMAX] ;

	// copy -l to buffer for parsing in ftclient
	memset(file, '\0', sizeof(file));
	strncat(file, LS, strlen(LS));

	// Open and loop through $PWD
	directory = opendir(".");
	if (directory) {
		while ((dFile = readdir(directory)) != NULL) {

			// if not current or parent directory
			if (strcmp(dFile->d_name, ".") == 0 || strcmp(dFile->d_name, "..") == 0) {
				continue;
			}
			// copy filename and newline to buffer
			strncat(file, dFile->d_name, strlen(dFile->d_name));
			strncat(file, "\n", 1);
		}
		closedir(directory);
		// Trim last newline
		file[strlen(file) - 1] = '\0';
		strncat(buffer, file, strlen(file));
		// printf("In getDir buff: %s\n", buffer);
		return 0;
	}
	return 1;
}

/***********************************************
** Function: Get File
** Descripton: Opens file, copies to buffer, and returns filesize
** Prerequisite: 
************************************************/
int getFile(char* buffer, char* fileName) {
	// Find file
    FILE* f = fopen(fileName, "r");
	if (f == NULL) {
		return -1;
	}
	// Get lenght of file
	fseek(f, 0, SEEK_END);
	int fsize = ftell(f);

	fseek(f, 0, SEEK_SET);

	fread(buffer, sizeof(char), fsize, f);
    fclose(f);

	return fsize;
}



/***********************************************
** Function: Receive Message
** Descripton: Receives message from chatserve
** Prerequisite: buffer is memset to null, bSize contains positive integer, flag contains positive integer. Called within child of spawnPid
************************************************/
char* recvMsg(int socketFD, char* buffer, int bSize, int flag) {
	int charsRead, charsTotal, count = 0; // stores recv() data, num of loops
	int size = -1;	// stores strlen(buffer) of server resposne
	char * ptr;		// stores msg portion of server response

	// While buffer doesn't contain endline
	while(size != charsTotal) {
		charsRead = recv(socketFD, buffer + charsTotal, bSize, flag);
		if (charsRead < 0) {
			error("Error from reading:");
		}
		// Parse <num bytes> from first packet
		if (count == 0) {
			ptr = strchr(buffer, ' ');
			size = atoi(buffer);
			*ptr++ = '\0';
			count++;
		}
		charsTotal += charsRead;	// add to offset next receive
	}
	// printf("ptr: %s\n", ptr);

	return ptr;
}

/***********************************************
** Function: Send Message
** Descripton: Sends message to ftclient
** Prerequisites: socket is open, buffer is memset to null, flag contains positive integer. Called within child of spawnPid
************************************************/
int sendMsg(int socketFD, char* buffer, int flag) {
		// Init variables
		char msg[MSGMAX]; 		// buffer for <num of bytes> + ' ' <msg>
		char sizeBStr[LENMAX];	// Stores <str(strlen(buffer))>
		char sizeStr[LENMAX];	// Stores <sizeBStr> + size + 1/2
		char len[LENMAX];		// stores result from sizeStr


		int charsWritten = 0;	// stores send() return
		int charsTotal = 0;		// stores send() returns
		memset(msg, '\0', sizeof(msg));
		memset(len, '\0', sizeof(len));
		memset(sizeBStr, '\0', sizeof(sizeBStr));
		memset(sizeStr, '\0', sizeof(sizeStr));

		// Buffer length to string
		int sizeB = strlen(buffer);
		sprintf(sizeBStr, "%d", sizeB);

		// sizeStr = buff length + sizeB + 1
		int size = strlen(sizeBStr) + sizeB + 1;
		sprintf(sizeStr, "%d", size);

		// Add buffer length string to be added to message.
		sprintf(len, "%d", size);

		// If (size+offset) length > (size), add 1 
		if (strlen(sizeStr) > strlen(sizeBStr)) {
			memset(len, '\0', sizeof(len));
			sprintf(len, "%d", (size+1));
		}

		// Copy len + " " + buffer to msg
		strcat(len, " ");
		strcat(msg, len);
		strcat(msg, buffer);
		
		// block until all data is sent
		while (charsTotal < strlen(msg)) {
			charsWritten = send(socketFD, msg + charsTotal, strlen(msg), 0);
			if (charsWritten < 0) {
				return -1;
			}
			if (charsWritten == 0) {
				break;
			}
			charsTotal += charsWritten; // add to offset next send
		}
	return charsTotal;
}

/***********************************************
** Function: Send File
** Descripton: Sends message to ftclient
** Prerequisites: socket is open, buffer is memset to null, flag contains positive integer. Called within child of spawnPid
************************************************/
int sendFile(int socketFD, char* buffer, char* fileName, int flag) {
	// Init variables
	char msg[MSGMAX]; 		// buffer for <num of bytes> + ' ' <msg>
	char sizeBStr[LENMAX];	// Stores <str(strlen(buffer))>
	char sizeStr[LENMAX];	// Stores <sizeBStr> + size + 1/2
	char len[LENMAX];		// stores result from sizeStr


	int charsWritten = 0;	// stores send() return
	int charsTotal = 0;		// stores send() returns
	memset(msg, '\0', sizeof(msg));
	memset(len, '\0', sizeof(len));
	memset(sizeBStr, '\0', sizeof(sizeBStr));
	memset(sizeStr, '\0', sizeof(sizeStr));

	// Find file
	FILE* f = fopen(fileName, "r");
	if (f == NULL) {
		return -1;
	}
	// Get lenght of file
	fseek(f, 0, SEEK_END);
	int fsize = ftell(f);
	fseek(f, 0, SEEK_SET);

	// Buffer length to string
	int sizeB = fsize;
	sprintf(sizeBStr, "%d", sizeB);

	// sizeStr = buff length + sizeB + 1
	int size = strlen(sizeBStr) + sizeB + 1;
	sprintf(sizeStr, "%d", size);

	// Add buffer length string to be added to message.
	sprintf(len, "%d", size);

	// If (size+offset) length > (size), add 1 
	if (strlen(sizeStr) > strlen(sizeBStr)) {
		memset(len, '\0', sizeof(len));
		sprintf(len, "%d", (size+1));
	}

	// Copy len + " " + buffer to msg
	strcat(len, " ");
	strcat(msg, len);
	
	// block until all data is sent
	int lseg, count, quit = 0;
    while (lseg = fread(buffer, 1, sizeof(buffer), f)) {
		// if end of file, read last segment and quit after send
		if (feof(f)) {
            fread(buffer, 1, lseg, f);
			quit = 1;
        }

		if (count == 0) {
			strcat(msg, buffer);
		} 
		else {
			memset(msg, '\0', sizeof(msg));
			strcat(msg, buffer);
		}

		// Send buffer to ftclient
		charsTotal = 0;
		while (charsTotal < strlen(msg)) {
			charsWritten = send(socketFD, msg + charsTotal, strlen(msg), 0);
			// If socket close on client side after last send
			if (charsWritten < 0) {
				return 1;
			}
			if (charsWritten == 0) {
				break;
			}
			charsTotal += charsWritten;
		}
		if (quit) {
			break;
		}
		memset(buffer, '\0', sizeof(buffer)); 
		count++;
    }
	fclose(f);
	return charsTotal;
}


/***********************************************
** Function: Get client Socket
** Descripton: initializes connection to ftclient data port
** Prerequisites: hostname and port are valid strings from ftclient recv()
************************************************/
int getClientSocket(char* hostName, char* portNumber) {
	struct hostent* serverHostInfo;		// stores host connection data
	int socketFD;
    struct addrinfo hints, *res;

	memset(&hints, 0, sizeof(hints));
	 
    getaddrinfo(hostName, portNumber, &hints, &res); 

	// Create socket from response
	socketFD = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (socketFD < 0) {
		error("ERROR:");
		return -1;
	}

	// Connect to server & socket to address
	if (connect(socketFD, res->ai_addr, res->ai_addrlen) < 0) {
		error("ERROR:");
		return -1;
	}
	return socketFD;
}

/***********************************************
** Function: Main
** Prerequisite: args[] contain:
**   {chatclient <machine> <server port> }
************************************************/
int main(int argc, char *argv[]) {
	// Call with ./client localhost <server port>
	int socketFD, clientSocket, portNumber, charsWritten, charsRead, charsTotal;
	int establishedConnectionFD;		// file descriptor to connection
	int listenSocketFD;					// socket file descriptor
	struct sockaddr_in serverAddress;	// stores socket address of host
	struct sockaddr_in clientAddress; // stores socket address of client

	socklen_t sizeOfClientInfo;	// Holds sizeof() client address
	char* ptr;
	char flip[6];
	char clientHost[34];				
	char clientPort[10];
	char fileName[INMAX];
	char buffer[INMAX];			// buffer for input
	char command[INMAX];

	pid_t spawnPid = -5;	// holds spawn child process

	// Check args == chatclient <machine> <server port>
	if (argc < 1) { 
		fprintf(stderr,"USAGE: %s hostname port\n", argv[0]);
		exit(1);
	}
	// Set up the server address struct
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear address struct
	portNumber = atoi(argv[1]); // Get the port number
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process

 	// Create the socket
	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0);
	if (listenSocketFD < 0) {
		error("ERROR opening socket");
	}
	// Enable the socket to begin listening & connect socket to port
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
		error("ERROR on binding");
	}
	// Flip the socket on - it can now receive up to 5 connections
	listen(listenSocketFD, 5);
	printf("Server is listening on port: %d\n", portNumber);

	while(1) {
		memset(clientHost, '\0', sizeof(clientHost));
		memset(clientPort, '\0', sizeof(clientPort));
		memset(fileName, '\0', sizeof(fileName));
		memset(flip, '\0', sizeof(flip));
		memset(buffer, '\0', sizeof(buffer)); // Clear Buffer
		memset(command, '\0', sizeof(command)); // Clear Buffer

		// Get the size of the address for the client that will connect
		sizeOfClientInfo = sizeof(clientAddress); 

		// Accept a connection
		establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
		if (establishedConnectionFD < 0) {
			error("ERROR on accept");
		}
		/******************************
		** Fork once handshake begins, child process will handle connection
		*******************************/
		spawnPid = fork();

		switch(spawnPid) {
			case -1:
				error("Hull Breach");
				break;

			/******************************
			** Child: handles connection from otp_enc
			*******************************/
			case 0:
			// Receive host + command from ftclient and close connection
			strcat(command, recvMsg(establishedConnectionFD, buffer, MSGMAX, 0));
			close(establishedConnectionFD);

			/******************************
			** If list directory
			*******************************/
			if (strncmp(command, LS, strlen(LS)) == 0) {
				// Parse -l <client host> <port>
				char* token = strtok(command, " ");
				int tcount = 1;
  				while (token != NULL) { 
					token = strtok(NULL, " "); 
					tcount++;
					if (tcount == 2) {
						strcat(flip, token);
					}
					else if (tcount == 3) {
						strcat(clientPort, token);
						strcat(clientHost, flip);
						strcat(clientHost, OSU);
					}
				}
				printf("Connection from %s\n", flip);
				printf("List directory requested on port %s\n", clientPort);

				// If correct number of args
				if (tcount == 4) {
					memset(buffer, '\0', sizeof(buffer)); 
					if (getDir(buffer) != 0) {
						printf("Buffer after getDir: %s\n", buffer);
						strcat(buffer, "-e ERROR UNABLE TO OPEN DIRECTORY");
					}
				}
				// If incorrect number of args
				else {
					strcat(buffer, "-e ERROR INVALID ARGS");
				}
				
				// Open connection to client data port, send, and close
				clientSocket = getClientSocket(clientHost, clientPort);
				sendMsg(clientSocket, buffer, 0);
				close(clientSocket);
			}
			/******************************
			** If Get file
			*******************************/
			else if (strncmp(command, GET, strlen(GET)) == 0) {
				// Parse -g <filename> <client host> <port>
				char* token = strtok(command, " ");
				int tcount = 1;
  				while (token != NULL) { 
					token = strtok(NULL, " "); 
					tcount++;
					if (tcount == 2) {
						strcpy(fileName, token);
					}
					if (tcount == 3) {
						strcat(clientPort, token);
					}
					else if (tcount == 4) {
						strcat(flip, token);
						strcat(clientHost, flip);
						strcat(clientHost, OSU);
					}
				}

				// Output connection
				printf("Connection from %s\n", flip);
				printf("File \"%s\" requested on port %s\n", fileName, flip);

				// Open connection to client data port
				clientSocket = getClientSocket(clientHost, clientPort);

				// If correct number of args
				if (tcount == 5) {
					memset(buffer, '\0', sizeof(buffer)); 
					// If file successfully sent to ftclient
					if (sendFile(clientSocket, buffer, fileName, 0) < 0) {
						printf("File not found. Sending error message to %s:%s\n", flip, clientPort);
						memset(buffer, '\0', sizeof(buffer)); 
						strcat(buffer, "-e FILE NOT FOUND");
						sendMsg(clientSocket, buffer, 0);
					} 
					else {
						printf("Sending File to %s:%s\n", flip, clientPort);
					}
				}
				else {
					strcat(buffer, "-e ERROR INVALID ARGS");
					sendMsg(clientSocket, buffer, 0);

				}
				// close socket
				close(clientSocket);
			}

			default:
				break;
		}
	}
	// Close the socket
	close(listenSocketFD);
	return 0;
}
