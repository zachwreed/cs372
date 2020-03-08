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

int getFile(char* fileName) {
	// Find file
    char buffer[1000];
    memset(buffer, '\0', sizeof(buffer)); 

    FILE* f = fopen(fileName, "r");
	if (f == NULL) {
        printf("Can't find file");
		return -1;
	}
	// Get lenght of file
	fseek(f, 0, SEEK_END);
	int fsize = ftell(f);

	fseek(f, 0, SEEK_SET);

	fread(buffer, sizeof(buffer), 1, f);
    int total = strlen(buffer);
    printf("%s", buffer);

    int lseg;
    while (lseg = fread(buffer, 1, sizeof(buffer), f)) {
        printf("%s", buffer);
        memset(buffer, '\0', sizeof(buffer)); 
        if (feof(f)) {
            fread(buffer, 1, lseg, f);
            printf("%s", buffer);
            break;
        }
    }

    fclose(f);

	return fsize;
}

int main() {
    printf("Get File pridePred.txt\n");
    getFile("pridePred.txt");
}
