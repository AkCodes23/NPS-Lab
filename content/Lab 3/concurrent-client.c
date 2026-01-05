// Client Side program to test
// the TCP server that returns
// a 'hi client' message

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

// PORT number
#define PORT 4444

int main()
{
	int clientSocket, ret;                                                           // Socket id
	struct sockaddr_in cliAddr, serverAddr;                                          // Client socket structure
	char buffer[1024];                                                               // char array to store incoming message

	clientSocket = socket(AF_INET,SOCK_STREAM, 0);                                   // Creating socket id

	if (clientSocket < 0) {
		printf("Error in connection.\n");
		exit(1);
	}
	printf("Client Socket is created.\n");

	memset(&cliAddr, '\0', sizeof(cliAddr));                                        // Initializing socket structure with NULL
	memset(buffer, '\0', sizeof(buffer));                                           // Initializing buffer array with NULL

	serverAddr.sin_family = AF_INET;                                                // Assigning port number and IP address
	serverAddr.sin_port = htons(PORT);

	serverAddr.sin_addr.s_addr= inet_addr("127.0.0.1");                             // 127.0.0.1 is Loopback IP
	ret = connect(clientSocket,(struct sockaddr*)&serverAddr,sizeof(serverAddr));   // connect() to connect to the server

	if (ret < 0) {
		printf("Error in connection.\n");
		exit(1);
	}

	printf("Connected to Server.\n");

	while (1) 
	{
		if (recv(clientSocket, buffer, 1024, 0) < 0)                            // recv() receives the message from server and stores in buffer
		{
			printf("Error in receiving data.\n");
		}
		else                                                                    // Printing the message on screen
		{
			printf("Server: %s\n", buffer);
			bzero(buffer, sizeof(buffer));
		}
	}

	return 0;
}

