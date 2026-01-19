#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080
#define MAX 200

int main()
{
    int sockfd;
    struct sockaddr_in serveraddr;
    char buffer[MAX];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(PORT);
    serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    connect(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));

    printf("Enter alphanumeric string: ");
    scanf("%s", buffer);

    write(sockfd, buffer, strlen(buffer));

    printf("\n--- Results from Server ---\n");

    while (read(sockfd, buffer, MAX) > 0) {
        printf("%s\n", buffer);
    }

    close(sockfd);
    return 0;
}
