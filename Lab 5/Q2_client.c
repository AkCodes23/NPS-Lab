#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define LINE_MAX 2048

static void trim_newline(char *s) {
    if (!s) return;
    size_t n = strlen(s);
    while (n > 0 && (s[n - 1] == '\n' || s[n - 1] == '\r')) {
        s[n - 1] = '\0';
        n--;
    }
}

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
    char tmp[LINE_MAX];
    snprintf(tmp, sizeof(tmp), "%s\n", line);
    return send_all(fd, tmp, strlen(tmp));
}

static int recv_line(int fd, char *out, size_t out_sz) {
    if (!out || out_sz == 0) return -1;
    size_t used = 0;
    while (used + 1 < out_sz) {
        char c;
        ssize_t rc = recv(fd, &c, 1, 0);
        if (rc < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        if (rc == 0) break;
        out[used++] = c;
        if (c == '\n') break;
    }
    out[used] = '\0';
    return (used == 0) ? 0 : 1;
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
    server.sin_port = htons(5002);
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

    char addr_line[128];
    snprintf(addr_line, sizeof(addr_line), "%s:%u", ip, port);

    char s1[LINE_MAX];
    char s2[LINE_MAX];

    printf("Enter string 1: ");
    fflush(stdout);
    if (!fgets(s1, sizeof(s1), stdin)) {
        close(fd);
        return 1;
    }

    printf("Enter string 2: ");
    fflush(stdout);
    if (!fgets(s2, sizeof(s2), stdin)) {
        close(fd);
        return 1;
    }

    trim_newline(s1);
    trim_newline(s2);

    // Send: client socket address, then two strings.
    send_line(fd, addr_line);
    send_line(fd, s1);
    send_line(fd, s2);

    char reply[LINE_MAX];
    if (recv_line(fd, reply, sizeof(reply)) > 0) {
        trim_newline(reply);
        printf("Server reply: %s\n", reply);
    } else {
        printf("Server reply: <no response>\n");
    }

    close(fd);
    return 0;
}
