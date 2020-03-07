/***********************************************
** Author: Zach Reed
** Description: Project 1 Chatclient
** Date: 2/9/2020
** References:
** 1.) 	Author: Benjamin Brewster
		Title:  Client Code for Module 4, CS344 Fall 2019
		Publisher: Oregon State
** 2.) 	Author: Zach Reed
		Title:  OTP Program 4 source code, CS344 Fall 2019
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

#define MSGMAX 514
#define INMAX 510
#define HANDLE 12
#define LENMAX 5
#define LS "-l "
#define GET "-g "


/***********************************************
** Function: Error Handler
** Description: Sends message to stderr and exits with status
************************************************/
void error(const char *msg) { 
	perror(msg); 
	exit(1);
} 


// https://stackoverflow.com/questions/4204666/how-to-list-files-in-a-directory-in-a-c-program
void getDir(char* directory) {
	  DIR *d;
  struct dirent *dir;
  d = opendir(directory);
  if (d) {
    while ((dir = readdir(d)) != NULL) {
      printf("%s\n", dir->d_name);
    }
    closedir(d);
  }
}


/***********************************************
** Function: Get File
** Descripton: Opens file, copies to buffer, and returns filesize
** Prerequisite: 
************************************************/
size_t getFile(char* buffer, char* fileName) {
	// Open file
    FILE* f = fopen(fileName, "r");
	if (f == NULL) {
		error("getFile:");
		exit(1);
	}
	// Get lenght of file
	fseek(f, 0, SEEK_END);
	size_t fsize = ftell(f);
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
	printf("ptr: %s\n", ptr);

	return ptr;
}

/***********************************************
** Function: Send Message
** Descripton: Sends message to chatserve
** Prerequisite: buffer is memset to null, bSize contains positive integer, flag contains positive integer. Called within child of spawnPid
************************************************/
int sendMsg(int socketFD, char* buffer, int flag) {
		// Init variables
		char msg[MSGMAX]; 		// buffer for <num of bytes> + ' ' <msg>
		char sizeBStr[LENMAX];	// Stores <str(strlen(buffer))>
		char sizeStr[LENMAX];	// Stores <sizeBStr> + size + 1/2
		char len[LENMAX];		// stores result from sizeStr
		char* ptr;		// stores msg portion of server response

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
		
		// printf("To client buffer: %s\n", buffer);
		// printf("To client len: %s\n", len);
		// printf("To Client msg: %s\n", msg);


		// block until all data is sent
		while (charsTotal < strlen(msg)) {
			charsWritten = send(socketFD, msg + charsTotal, strlen(msg), 0);
			if (charsWritten < 0) {
				error("CLIENT: ERROR writing to socket");
			}
			if (charsWritten == 0) {
				break;
			}
			charsTotal += charsWritten; // add to offset next send
		}
	return charsTotal;
}

/***********************************************
** Function: Main
** Prerequisite: args[] contain:
**   {chatclient <machine> <server port> }
************************************************/
int main(int argc, char *argv[]) {
	// Call with ./client localhost <server port>
	int socketFD, portNumber, charsWritten, charsRead, charsTotal;
	int establishedConnectionFD;		// file descriptor to connection
	int listenSocketFD;					// socket file descriptor
	struct sockaddr_in serverAddress;	// stores socket address of host
	struct sockaddr_in clientAddress; // stores socket address of client

	socklen_t sizeOfClientInfo;			// Holds sizeof() client address
	char* ptr;		// stores msg portion of server response
	char buffer[INMAX];				// buffer for input
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
	printf("Port number = %d\n", portNumber);
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
		// Get the size of the address for the client that will connect
		sizeOfClientInfo = sizeof(clientAddress); 

		// Accept a connection
		establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
		if (establishedConnectionFD < 0) {
			printf("%d\n",establishedConnectionFD);
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
			// Receive command from ftclient
			memset(buffer, '\0', sizeof(buffer)); // Clear Buffer
			memset(command, '\0', sizeof(command)); // Clear Buffer
			strcat(command, recvMsg(establishedConnectionFD, buffer, MSGMAX, 0));

			printf("From Client: %s\n", command);
			memset(buffer, '\0', sizeof(buffer)); 

			// Do something with user command
			if (strncmp(command, LS, strlen(LS)) == 0) {
				
				ptr = strchr(command, ' ');
				*ptr++ = '\0';
				printf("Requested ls of dir: %s\n", ptr);


				memset(buffer, '\0', sizeof(buffer)); 
				strcat(buffer, "Fill with Directory");

			}
			else if (strncmp(command, GET, strlen(GET)) == 0) {
				ptr = strchr(command, ' ');
				*ptr++ = '\0';

				printf("Requested File: %s\n", ptr);
				strcat(buffer, "Fill with File");
			}
			else {
				printf("Client Error\n");
				memset(buffer, '\0', sizeof(buffer)); 
				strcat(buffer, "Invalid command");
			}

			sendMsg(establishedConnectionFD, buffer, 0);

			// Do something with response

			// Send message to server
			// sendMsg(socketFD, buffer, 0);

			default:
				close(establishedConnectionFD);
				break;
		}
	}
	// Close the socket
	close(listenSocketFD);
	return 0;
}
