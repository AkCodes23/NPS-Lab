#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define DEFAULT_PORT 5555
#define MAX_LINE 4096

static void trim_newline(char *s)
{
    size_t n = strlen(s);
    while (n > 0 && (s[n - 1] == '\n' || s[n - 1] == '\r')) {
        s[n - 1] = '\0';
        n--;
    }
}

int main(int argc, char **argv)
{
    const char *host = "127.0.0.1";
    int port = DEFAULT_PORT;

    if (argc >= 2) {
        host = argv[1];
    }
    if (argc >= 3) {
        port = atoi(argv[2]);
        if (port <= 0 || port > 65535) {
            fprintf(stderr, "Invalid port: %s\n", argv[2]);
            return 1;
        }
    }

    char input[MAX_LINE];
    memset(input, 0, sizeof(input));

    if (argc >= 4) {
        snprintf(input, sizeof(input), "%s", argv[3]);
    } else {
        printf("Enter an alphanumeric string: ");
        if (!fgets(input, sizeof(input), stdin)) {
            fprintf(stderr, "Failed to read input\n");
            return 1;
        }
        trim_newline(input);
    }

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons((uint16_t)port);

    if (inet_pton(AF_INET, host, &serveraddr.sin_addr) != 1) {
        fprintf(stderr, "Invalid IPv4 address: %s\n", host);
        close(sockfd);
        return 1;
    }

    if (connect(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0) {
        perror("connect");
        close(sockfd);
        return 1;
    }

    // Send input line terminated by '\n'
    char sendbuf[MAX_LINE + 2];
    snprintf(sendbuf, sizeof(sendbuf), "%s\n", input);

    if (send(sockfd, sendbuf, strlen(sendbuf), 0) < 0) {
        perror("send");
        close(sockfd);
        return 1;
    }

    // Receive and print until server closes connection
    char recvbuf[MAX_LINE];
    while (1) {
        ssize_t n = recv(sockfd, recvbuf, sizeof(recvbuf) - 1, 0);
        if (n == 0) {
            break;
        }
        if (n < 0) {
            perror("recv");
            close(sockfd);
            return 1;
        }
        recvbuf[n] = '\0';
        fputs(recvbuf, stdout);
    }

    close(sockfd);
    return 0;
}
