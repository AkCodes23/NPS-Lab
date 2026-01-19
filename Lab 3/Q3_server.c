#include <arpa/inet.h>
#include <ctype.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define DEFAULT_PORT 5555
#define MAX_LINE 4096

static int cmp_char_asc(const void *a, const void *b)
{
    const unsigned char ca = *(const unsigned char *)a;
    const unsigned char cb = *(const unsigned char *)b;
    return (ca > cb) - (ca < cb);
}

static int cmp_char_desc(const void *a, const void *b)
{
    return -cmp_char_asc(a, b);
}

static ssize_t read_line(int fd, char *buf, size_t maxlen)
{
    size_t i = 0;
    while (i + 1 < maxlen) {
        char c;
        ssize_t n = recv(fd, &c, 1, 0);
        if (n == 0) {
            break; // EOF
        }
        if (n < 0) {
            return -1;
        }
        if (c == '\n') {
            break;
        }
        buf[i++] = c;
    }
    buf[i] = '\0';
    return (ssize_t)i;
}

static void build_sorted_digits_asc(const char *input, char *out, size_t outsz)
{
    char digits[MAX_LINE];
    size_t n = 0;

    for (size_t i = 0; input[i] != '\0' && n + 1 < sizeof(digits); i++) {
        unsigned char c = (unsigned char)input[i];
        if (isdigit(c)) {
            digits[n++] = (char)c;
        }
    }
    digits[n] = '\0';

    qsort(digits, n, sizeof(digits[0]), cmp_char_asc);

    if (n == 0) {
        snprintf(out, outsz, "(no digits)");
        return;
    }

    snprintf(out, outsz, "%s", digits);
}

static void build_sorted_chars_desc(const char *input, char *out, size_t outsz)
{
    char letters[MAX_LINE];
    size_t n = 0;

    for (size_t i = 0; input[i] != '\0' && n + 1 < sizeof(letters); i++) {
        unsigned char c = (unsigned char)input[i];
        if (isalpha(c)) {
            letters[n++] = (char)c;
        }
    }
    letters[n] = '\0';

    qsort(letters, n, sizeof(letters[0]), cmp_char_desc);

    if (n == 0) {
        snprintf(out, outsz, "(no letters)");
        return;
    }

    snprintf(out, outsz, "%s", letters);
}

int main(int argc, char **argv)
{
    int port = DEFAULT_PORT;
    if (argc >= 2) {
        port = atoi(argv[1]);
        if (port <= 0 || port > 65535) {
            fprintf(stderr, "Invalid port: %s\n", argv[1]);
            return 1;
        }
    }

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return 1;
    }

    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = INADDR_ANY;
    serveraddr.sin_port = htons((uint16_t)port);

    if (bind(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0) {
        perror("bind");
        close(sockfd);
        return 1;
    }

    if (listen(sockfd, 5) < 0) {
        perror("listen");
        close(sockfd);
        return 1;
    }

    printf("Q3 server listening on port %d...\n", port);

    struct sockaddr_in clientaddr;
    socklen_t clientlen = sizeof(clientaddr);
    int clientfd = accept(sockfd, (struct sockaddr *)&clientaddr, &clientlen);
    if (clientfd < 0) {
        perror("accept");
        close(sockfd);
        return 1;
    }

    printf("Client connected from %s:%d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

    char input[MAX_LINE];
    if (read_line(clientfd, input, sizeof(input)) < 0) {
        perror("recv");
        close(clientfd);
        close(sockfd);
        return 1;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        close(clientfd);
        close(sockfd);
        return 1;
    }

    if (pid == 0) {
        // Child: sort digits ascending and send result with PID
        char digits_sorted[MAX_LINE];
        build_sorted_digits_asc(input, digits_sorted, sizeof(digits_sorted));

        char msg[MAX_LINE];
        int len = snprintf(msg, sizeof(msg), "Child PID %d: Digits ascending = %s\n", (int)getpid(), digits_sorted);
        if (len > 0) {
            (void)send(clientfd, msg, (size_t)len, 0);
        }

        close(clientfd);
        _exit(0);
    }

    // Parent: wait for child to finish sending to avoid interleaved writes
    (void)waitpid(pid, NULL, 0);

    char chars_sorted[MAX_LINE];
    build_sorted_chars_desc(input, chars_sorted, sizeof(chars_sorted));

    char msg[MAX_LINE];
    int len = snprintf(msg, sizeof(msg), "Parent PID %d: Letters descending = %s\n", (int)getpid(), chars_sorted);
    if (len > 0) {
        (void)send(clientfd, msg, (size_t)len, 0);
    }

    close(clientfd);
    close(sockfd);
    return 0;
}
