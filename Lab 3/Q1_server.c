#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8080
#define MAX 100

int main()
{
    int sockfd, newsockfd;
    struct sockaddr_in serveraddr, clientaddr;
    socklen_t len;
    char buf[MAX];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(1);
    }

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = INADDR_ANY;
    serveraddr.sin_port = htons(PORT);

    bind(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
    listen(sockfd, 1);

    printf("Server waiting for connection...\n");

    len = sizeof(clientaddr);
    newsockfd = accept(sockfd, (struct sockaddr *)&clientaddr, &len);

    printf("Client connected\n");

    pid_t pid = fork();

    if (pid > 0) {
        /* Parent process: Send */
        printf("\n[Server Parent] PID=%d PPID=%d\n", getpid(), getppid());
        while (1) {
            printf("Server: ");
            fgets(buf, MAX, stdin);
            write(newsockfd, buf, strlen(buf));
        }
    } 
    else {
        /* Child process: Receive */
        printf("\n[Server Child] PID=%d PPID=%d\n", getpid(), getppid());
        while (1) {
            memset(buf, 0, MAX);
            read(newsockfd, buf, MAX);
            printf("Client: %s", buf);
        }
    }

    close(newsockfd);
    close(sockfd);
    return 0;
}
