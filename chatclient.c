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
int recvMsg(int connectionFD, char* buffer, int bSize, int flag) {
	int charsRead, charsTotal, count = 0;
	int size = -1;
	char * ptr;

	// While buffer doesn't contain endline
	while(size != charsTotal) {
		charsRead = recv(connectionFD, buffer + charsTotal, bSize, flag);
		// printf("Before> %s\n", buffer);
		if (charsRead < 0) {
			perror("Error from reading:");
			exit(1);
		}
		if (count == 0) {
			ptr = strchr(buffer, ' ');
			size = atoi(buffer);
			*ptr++ = '\0';
			count++;
		}
		charsTotal += charsRead;	// add to offset next receive
	}

	if (strcmp(ptr, "\\quit") == 0) {
		return -1;
	}

	printf("Server> %s\n", ptr);
	return charsTotal;
}


/***********************************************
** Function: Send Message
** Descripton: Sends message to chatserve
** Prerequisite: buffer is memset to null, bSize contains positive integer, flag contains positive integer. Called within child of spawnPid
************************************************/
int sendMsg(int socketFD, char* buffer, int flag) {
		char msg[MSGMAX]; 					// buffer for <num of bytes> + ' ' <msg>
		char sizeBStr[LENMAX];
		char sizeStr[LENMAX];
		char len[LENMAX];
		memset(msg, '\0', sizeof(msg));
		memset(len, '\0', sizeof(len));
		memset(sizeBStr, '\0', sizeof(sizeBStr));
		memset(sizeStr, '\0', sizeof(sizeStr));

		// printf("after setup and memset\n");

		// Buffer length to string
		int sizeB = strlen(buffer);
		sprintf(sizeBStr, "%d", sizeB);

				// printf("after buffer len to string\n");

		// sizeStr = buff length + sizeB + 1
		int size = strlen(sizeBStr) + sizeB + 1;
		sprintf(sizeStr, "%d", size);

				// printf("after size to string\n");

		// Add buffer length string to be added to message.
		sprintf(len, "%d", size);
		// If (size+offset) length > (size), add 1 
		if (strlen(sizeStr) > strlen(sizeBStr)) {
			memset(len, '\0', sizeof(len));
			sprintf(len, "%d", (size+1));
		}
				// printf("after copy size to len\n");

		strcat(len, " ");
		strcat(msg, len);
		strcat(msg, buffer);
		

		int charsWritten = 0;
		int charsTotal = 0;
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
**   {'chatclient', serverport }
************************************************/
int main(int argc, char *argv[])
{
	// Call with ./client localhost <server port>
	int socketFD, portNumber, charsWritten, charsRead, charsTotal;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	char buffer[INMAX]; 
	char msg[MSGMAX]; 	// buffer for <num of bytes> + ' ' <msg>
	char *handle = malloc(sizeof(char) * HANDLE);


	if (argc < 3) { 
		fprintf(stderr,"USAGE: %s hostname port\n", argv[0]); 
		exit(0); 
	}

	// Set up the server address struct
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[2]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverHostInfo = gethostbyname(argv[1]); // Convert the machine name into a special form of address
	if (serverHostInfo == NULL) { 
		fprintf(stderr, "CLIENT: ERROR, no such host\n"); 
		exit(0); 
		}
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address

	// Set up the socket
	
	socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (socketFD < 0) error("CLIENT: ERROR opening socket");
	
	// Connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to address
		error("CLIENT: ERROR connecting");

	// Get Handle from User
	handle = getHandle();
	int sendN = 0;
	int quit = 0;
	while(quit != 1) {
		fflush(stdin);

		if (sendN == 0) {
			sendMsg(socketFD, handle, 0);
			sendN++;
			continue;
		}

		printf("%s> ", handle);
		memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer array
		memset(msg, '\0', sizeof(msg));
		fgets(buffer, sizeof(buffer) - 1, stdin); // Get input from the user, trunc to buffer - 1 chars, leaving \0
		buffer[strcspn(buffer, "\n")] = '\0'; // Remove the trailing \n that fgets adds
		
		// If quit, send quit to server and break loop
		if (strcmp(buffer, "\\quit") == 0) {
			sendMsg(socketFD, buffer, 0);
			break;
		} 

		sendMsg(socketFD, buffer, 0);
		sendN++;

		// Get return message from server
		memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
		charsTotal = recvMsg(socketFD, buffer, MSGMAX, 0);

		if (charsTotal < 0) {
			break;
		}
	}
	close(socketFD); // Close the socket
	return 0;
}
