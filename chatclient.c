#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <math.h>
#include <errno.h>

#define MSGMAX 504
#define INMAX 500
#define HANDLE 12
#define LENMAX 5

/***********************************************
** Function: Error Handler
** Description: Sends message to stderr and exits with status
************************************************/
void error(const char *msg) { 
	perror(msg); 
	exit(0); 
} 

/***********************************************
** Function: Receive Message
** Descripton: Receives message from chatserve
** Prerequisite: buffer is memset to null, bSize contains positive integer, flag contains positive integer. Called within child of spawnPid
************************************************/
int recvMsg(int socketFD, char* buffer, int bSize, int flag) {
	int charsRead, charsTotal, count = 0; // stores recv() data, num of loops
	int size = -1;	// stores strlen(buffer) of server resposne
	char * ptr;		// stores msg portion of server response

	// While buffer doesn't contain endline
	while(size != charsTotal) {
		charsRead = recv(socketFD, buffer + charsTotal, bSize, flag);
		if (charsRead < 0) {
			perror("Error from reading:");
			exit(1);
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
	// If server reponses with quit, return negative value
	if (strcmp(ptr, "\\quit") == 0) {
		return -1;
	}
	// Print server response
	printf("Server> %s\n", ptr);
	return charsTotal;
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
** Function: Get Handle
** Description: Prompts user for chat handle to use during session
** Prerequisite: None
************************************************/
char* getHandle() {
	char *handle = (char*)malloc(HANDLE);

	printf("Enter a handle up to 10 characters: ");
	fgets(handle, HANDLE - 1, stdin); // Get input from the user, trunc to buffer - 1 chars, leaving \0
	handle[strcspn(handle, "\n")] = '\0'; // Remove the trailing \n that fgets adds
	return handle;
}

/***********************************************
** Function: Main
** Prerequisite: args[] contains:
**   {'chatclient', machine, serverport }
************************************************/
int main(int argc, char *argv[])
{
	// Call with ./client localhost <server port>
	int socketFD, portNumber, charsWritten, charsRead, charsTotal;
	struct sockaddr_in serverAddress;	// stores socket address data
	struct hostent* serverHostInfo;		// stores host connection data
	char buffer[INMAX]; 				// buffer for input
	char msg[MSGMAX]; 					// buffer for <bufer strlen> + ' ' <buffer>
	char *handle = malloc(sizeof(char) * HANDLE);	// stores client handle

	// Check args == chatclient <machine> <server port>
	if (argc < 3) { 
		fprintf(stderr,"USAGE: %s hostname port\n", argv[0]); 
		exit(0); 
	}

	// Set up the server address struct
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear address struct
	portNumber = atoi(argv[2]); // Get the port number
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverHostInfo = gethostbyname(argv[1]); // Convert the machine name into address
	if (serverHostInfo == NULL) { 
		fprintf(stderr, "CLIENT: ERROR, no such host\n"); 
		exit(0); 
	}

	// Copy the address
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length);

	// Set up the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (socketFD < 0) {
		error("CLIENT: ERROR opening socket");
	}

	// Connect to server & socket to address
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
		error("CLIENT: ERROR connecting");
	}

	// Get Handle from User
	handle = getHandle();
	// Init variables for loop
	int sendN = 0;
	int quit = 0;

	/******************
	 * Chat loop
	 *****************/
	while(quit != 1) {
		fflush(stdin); // clear stdin

		// If new session, send handle to server
		if (sendN == 0) {
			sendMsg(socketFD, handle, 0);
			sendN++;
			continue;
		}

		// Get user Input 
		printf("%s> ", handle);					
		memset(buffer, '\0', sizeof(buffer)); 		// Clear Buffer
		memset(msg, '\0', sizeof(msg));				// Clear Msg
		fgets(buffer, sizeof(buffer) - 1, stdin); 	// Get input from the user, trunc to buffer - 1 chars, leaving \0
		buffer[strcspn(buffer, "\n")] = '\0'; 		// Remove the trailing \n
		
		// If quit input, send quit to server and break loop
		if (strcmp(buffer, "\\quit") == 0) {
			sendMsg(socketFD, buffer, 0);
			break;
		} 
		// Send message to server
		sendMsg(socketFD, buffer, 0);
		sendN++;

		// Get return message from server
		memset(buffer, '\0', sizeof(buffer)); // Clear Buffer
		charsTotal = recvMsg(socketFD, buffer, MSGMAX, 0);
		// If '\quit' from server
		if (charsTotal < 0) {
			break;
		}
	}
	// Close the socket
	close(socketFD);
	return 0;
}
