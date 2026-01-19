#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

static int send_all(int fd, const char *buf, size_t len) {
    size_t sent = 0;
    while (sent < len) {
        ssize_t rc = send(fd, buf + sent, len - sent, 0);
        if (rc < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        if (rc == 0) return -1;
        sent += (size_t)rc;
    }
    return 0;
}

static int send_line(int fd, const char *line) {
    char tmp[1024];
    snprintf(tmp, sizeof(tmp), "%s\n", line);
    return send_all(fd, tmp, strlen(tmp));
}

int main(void) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in server;
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(5001);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(fd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("connect");
        close(fd);
        return 1;
    }

    struct sockaddr_in local;
    socklen_t local_len = sizeof(local);
    if (getsockname(fd, (struct sockaddr *)&local, &local_len) < 0) {
        perror("getsockname");
        close(fd);
        return 1;
    }

    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &local.sin_addr, ip, sizeof(ip));
    unsigned port = (unsigned)ntohs(local.sin_port);

    const char *keyword = "Technology";
    char msg[256];
    snprintf(msg, sizeof(msg), "%s|%s:%u", keyword, ip, port);
    send_line(fd, msg);

    // Wait for "terminate session" broadcast.
    char buf[256];
    while (1) {
        ssize_t n = recv(fd, buf, sizeof(buf) - 1, 0);
        if (n <= 0) break;
        buf[n] = '\0';
        if (strstr(buf, "terminate session") != NULL) {
            printf("terminate session\n");
            break;
        }
    }

    close(fd);
    return 0;
}
