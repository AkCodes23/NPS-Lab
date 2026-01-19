// Server side program that sends
// a 'hi client' message
// to every client concurrently

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
	
	int sockfd, ret;                                            // Server socket id
	struct sockaddr_in serverAddr;                              // Server socket address structures
	int clientSocket;                                           // Client socket id

	struct sockaddr_in cliAddr;                                 // Client socket address structures

	socklen_t addr_size;                                        // Stores byte size of server socket address

	pid_t childpid;                                             // Child process id

	sockfd = socket(AF_INET, SOCK_STREAM, 0);                   // Creates a TCP socket id from IPV4 family

	if (sockfd < 0) {                                           // Error handling if socket id is not valid
		printf("Error in connection.\n");
		exit(1);
	}

	printf("Server Socket is created.\n");
	memset(&serverAddr, '\0', sizeof(serverAddr));              // Initializing address structure with NULL
	serverAddr.sin_family = AF_INET;                            // Assign port number and IP address to the socket created
	serverAddr.sin_port = htons(PORT);
	serverAddr.sin_addr.s_addr= inet_addr("127.0.0.1");         // 127.0.0.1 is a loopback address

	ret = bind(sockfd,(struct sockaddr*)&serverAddr,sizeof(serverAddr)); // Binding the socket id with the socket structure

	if (ret < 0) 
	{                                              // Error handling
		printf("Error in binding.\n");
		exit(1);
	}

	if (listen(sockfd, 10) == 0)                               // Listening for connections (upto 10)
	{
		printf("Listening...\n\n");
	}

	int cnt = 0;
	while (1) 
	{
		clientSocket = accept(sockfd, (struct sockaddr*)&cliAddr,&addr_size);  // Accept clients and store their information in cliAddr

		if (clientSocket < 0)                              // Error handling
		{
			exit(1);
		}

		printf("Connection accepted from %s:%d\n",inet_ntoa(cliAddr.sin_addr), ntohs(cliAddr.sin_port));  // Displaying information of connected client
		printf("Clients connected: %d\n\n",++cnt);        // Print number of clients connected till now

		if ((childpid = fork()) == 0)                     // Creates a child process
		{
			close(sockfd);                            // Closing the server socket id
			send(clientSocket, "hi client", strlen("hi client"), 0);  // Send a confirmation message to the client
		}
	}
	close(clientSocket);                                     // Close the client socket id
	return 0;
}

