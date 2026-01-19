#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8080
#define MAX 1024

/* Function to swap characters */
void swap(char *a, char *b)
{
    char temp = *a;
    *a = *b;
    *b = temp;
}

/* Recursive function to generate permutations */
void permute(char *str, int l, int r, char *result)
{
    int i;
    if (l == r) {
        strcat(result, str);
        strcat(result, "\n");
    } else {
        for (i = l; i <= r; i++) {
            swap(&str[l], &str[i]);
            permute(str, l + 1, r, result);
            swap(&str[l], &str[i]); // backtrack
        }
    }
}

int main()
{
    int sockfd;
    char buffer[MAX], result[MAX * 10];
    struct sockaddr_in serveraddr, clientaddr;
    socklen_t len;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = INADDR_ANY;
    serveraddr.sin_port = htons(PORT);

    bind(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));

    printf("UDP Server waiting for string...\n");

    len = sizeof(clientaddr);
    recvfrom(sockfd, buffer, MAX, 0,
             (struct sockaddr *)&clientaddr, &len);

    printf("Received string from client: %s\n", buffer);

    result[0] = '\0';
    permute(buffer, 0, strlen(buffer) - 1, result);

    sendto(sockfd, result, strlen(result), 0,
           (struct sockaddr *)&clientaddr, len);

    printf("Permutations sent to client\n");

    close(sockfd);
    return 0;
}
