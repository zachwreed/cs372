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
#define HANDLE 16

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
		printf("Server: %s\n", buffer);
		if (charsRead < 0) {
			printf("ERROR reading from socket %d\n", charsRead);
			perror("Error from reading:");
			exit(1);
		}
		if (count == 0) {
			ptr = strchr(buffer, ' ');
			size = atoi(buffer);
			*ptr++ = '\0';
			count++;
			printf("Server: %s\n", ptr);
		}
		charsTotal += charsRead;	// add to offset next receive
	}
	printf("Server: %s\n", ptr);
	return charsTotal;
}


/***********************************************
** Function: Send Message
** Descripton: Sends message to chatserve
** Prerequisite: buffer is memset to null, bSize contains positive integer, flag contains positive integer. Called within child of spawnPid
************************************************/
int sendMsg(int connectionFD, char* buffer, int flag) {
	int charsWritten, charsTotal = 0;
	// block until all data is sent
	while (charsTotal < strlen(buffer)) {
		charsWritten = send(connectionFD, buffer + charsTotal, strlen(buffer), flag);
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
	char *handle = malloc(sizeof(char) * HANDLE);

	printf("Enter a handle up to 15 characters: ");
	fgets(handle, sizeof(handle) - 1, stdin); // Get input from the user, trunc to buffer - 1 chars, leaving \0
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
	int socketFD, portNumber, charsWritten, charsRead;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	char buffer[INMAX]; 
	char msg[MSGMAX]; 	// buffer for <num of bytes> + ' ' <msg>
	char *handle = malloc(sizeof(char) * HANDLE);
	if (argc < 3) { 
		fprintf(stderr,"USAGE: %s hostname port\n", argv[0]); 
		exit(0); 
	} // Check usage & args

	// Set up the server address struct
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[2]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverHostInfo = gethostbyname(argv[1]); // Convert the machine name into a special form of address
	if (serverHostInfo == NULL) { fprintf(stderr, "CLIENT: ERROR, no such host\n"); exit(0); }
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address

	// Set up the socket
	
	socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (socketFD < 0) error("CLIENT: ERROR opening socket");
	
	// Connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to address
		error("CLIENT: ERROR connecting");

	// Get Handle from User
	handle = getHandle();
	int quit = 0;
	while(quit != 1) {
		printf("%s> ", handle);
		memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer array
		memset(msg, '\0', sizeof(msg));
		fgets(buffer, sizeof(buffer) - 1, stdin); // Get input from the user, trunc to buffer - 1 chars, leaving \0
		buffer[strcspn(buffer, "\n")] = '\0'; // Remove the trailing \n that fgets adds
		
		if (strcmp(buffer, "\\quit") == 0) {
			quit = 1;
		} 
		
		int size = floor(log10(abs((int)strlen(buffer))))+2;
		char len[size];
		sprintf(len, "%ld", strlen(buffer)+size);
		strcat(len, " ");
		// printf("Size: %d\n", size);
		strcat(msg, len);
		strcat(msg, buffer);

		charsWritten = 0;
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

		if(quit == 1) {
			break;
		}

		// // Send message to server
		// charsWritten = send(socketFD, msg, strlen(msg), 0); // Write to the server
		// if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
		// if (charsWritten < strlen(buffer)) printf("CLIENT: WARNING: Not all data written to socket!\n");

		// Get return message from server
		memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
		
		int count = 0;
		charsRead = 0;
		charsTotal = 0;
		int sizeB = -1;
		char * ptr;

		// Receive message from server
		while(sizeB != charsTotal) {
			charsRead = recv(socketFD, buffer + charsTotal, sizeof(buffer)-1, 0);
			if (charsRead < 0) {
				perror("Error reading from socket");
				exit(1);
			}
			if (count == 0) {
				ptr = strchr(buffer, ' ');
				sizeB = atoi(buffer);
				*ptr++ = '\0';
				count++;
			}
			charsTotal += charsRead;	// add to offset next receive
		}
		if (strcmp(ptr, "\\quit") == 0) {
			quit = 1;
		}
		else { 
			printf("Serv> %s\n", ptr);
		}
			
		// charsRead = recvMsg(socketFD, buffer, MSGMAX, 0);
		// if (charsRead < 0) error("CLIENT: ERROR reading from socket");
		// printf("CLIENT: I received this from the server: \"%s\"\n", buffer);
	}
	close(socketFD); // Close the socket
	return 0;
}
