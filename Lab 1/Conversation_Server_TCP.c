#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
 
#define MAXSIZE 1024
#define PORT 3388
 
int main() {
    int sockfd, newsockfd;
    socklen_t actuallen;
    struct sockaddr_in serveraddr, clientaddr;
    char buff[MAXSIZE];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    printf("[INFO] Socket created.\n");

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(PORT);
    serveraddr.sin_addr.s_addr = INADDR_ANY;
 
    if (bind(sockfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) == -1) {
        perror("Bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    printf("[INFO] Socket bound to port %d.\n", PORT);

    if (listen(sockfd, 1) == -1) {
        perror("Listen failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    printf("[INFO] Listening on port %d...\n", PORT);
 
    actuallen = sizeof(clientaddr);

    newsockfd = accept(sockfd, (struct sockaddr*)&clientaddr, &actuallen);
    if (newsockfd == -1) {
        perror("Accept failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    printf("[INFO] Client connected!\n");
 
    while (1) {
        memset(buff, 0, sizeof(buff));
 
        // Receive message
        int recedbytes = recv(newsockfd, buff, sizeof(buff) - 1, 0);
        if (recedbytes <= 0) {
            printf("[INFO] Client disconnected.\n");
            break;
        }
        buff[recedbytes] = '\0';
        printf("Client: %s", buff);
        fflush(stdout);
 
        // Stop if client sends "stop"
        if (strncmp(buff, "stop", 4) == 0) {
            printf("[INFO] Stop received from client. Exiting.\n");
            break;
        }
 
        // Send reply
        printf("Server: ");
        if (!fgets(buff, sizeof(buff), stdin)) break;
 
        // Send newline-terminated message
        size_t len = strlen(buff);
        if (buff[len - 1] != '\n') {
            buff[len] = '\n';
            buff[len + 1] = '\0';
            len++;
        }
 
        // Send all bytes
        size_t total = 0;
        while (total < len) {
            int n = send(newsockfd, buff + total, len - total, 0);
            if (n <= 0) {
                perror("Send failed");
                goto end;
            }
            total += n;
        }
 
        // Stop if server sends "stop"
        if (strncmp(buff, "stop", 4) == 0) {
            printf("[INFO] Stop sent. Exiting.\n");
            break;
        }
    }
 
end:
    close(newsockfd);
    close(sockfd);
    printf("[INFO] Server closed.\n");
    return 0;
}