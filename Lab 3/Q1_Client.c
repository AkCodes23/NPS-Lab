#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080
#define MAX 100

int main()
{
    int sockfd;
    struct sockaddr_in serveraddr;
    char buf[MAX];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(1);
    }

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(PORT);
    serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    connect(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
    printf("Connected to server\n");

    pid_t pid = fork();

    if (pid > 0) {
        /* Parent process: Send */
        printf("\n[Client Parent] PID=%d PPID=%d\n", getpid(), getppid());
        while (1) {
            printf("Client: ");
            fgets(buf, MAX, stdin);
            write(sockfd, buf, strlen(buf));
        }
    } 
    else {
        /* Child process: Receive */
        printf("\n[Client Child] PID=%d PPID=%d\n", getpid(), getppid());
        while (1) {
            memset(buf, 0, MAX);
            read(sockfd, buf, MAX);
            printf("Server: %s", buf);
        }
    }

    close(sockfd);
    return 0;
}
