#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080
#define MAX 1024

int main()
{
    int sockfd;
    char buffer[MAX];
    struct sockaddr_in serveraddr;
    socklen_t len;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(PORT);
    serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    printf("Enter a string: ");
    scanf("%s", buffer);

    /* Client sends first (Half duplex) */
    sendto(sockfd, buffer, strlen(buffer), 0,
           (struct sockaddr *)&serveraddr, sizeof(serveraddr));

    /* Client receives permutations */
    len = sizeof(serveraddr);
    recvfrom(sockfd, buffer, MAX, 0,
             (struct sockaddr *)&serveraddr, &len);

    printf("\nPermutations received from server:\n%s", buffer);

    close(sockfd);
    return 0;
}
