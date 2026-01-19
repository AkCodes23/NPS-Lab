#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
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

static int cmp_uchar(const void *a, const void *b) {
    unsigned char x = *(const unsigned char *)a;
    unsigned char y = *(const unsigned char *)b;
    return (x > y) - (x < y);
}

// Simplest practical interpretation: anagram ignoring spaces and case.
static void normalize_for_anagram(const char *in, unsigned char **out, size_t *out_len) {
    *out = NULL;
    *out_len = 0;
    if (!in) return;

    size_t cap = strlen(in);
    unsigned char *buf = (unsigned char *)malloc(cap + 1);
    if (!buf) return;

    size_t used = 0;
    for (size_t i = 0; in[i] != '\0'; i++) {
        unsigned char c = (unsigned char)in[i];
        if (c == ' ' || c == '\t') continue;
        buf[used++] = (unsigned char)tolower(c);
    }

    qsort(buf, used, sizeof(unsigned char), cmp_uchar);
    *out = buf;
    *out_len = used;
}

static int are_anagrams(const char *a, const char *b) {
    unsigned char *na = NULL;
    unsigned char *nb = NULL;
    size_t la = 0, lb = 0;

    normalize_for_anagram(a, &na, &la);
    normalize_for_anagram(b, &nb, &lb);

    int ok = 0;
    if (na && nb && la == lb && (la == 0 || memcmp(na, nb, la) == 0)) ok = 1;

    free(na);
    free(nb);
    return ok;
}

static void print_datetime_with_addr(const char *client_addr_str) {
    time_t now = time(NULL);
    struct tm local_tm;
    localtime_r(&now, &local_tm);

    char ts[64];
    strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", &local_tm);

    printf("[%s] Connected client socket address: %s\n", ts, client_addr_str);
    fflush(stdout);
}

static void handle_client(int cfd) {
    char addr_line[LINE_MAX];
    char s1[LINE_MAX];
    char s2[LINE_MAX];

    if (recv_line(cfd, addr_line, sizeof(addr_line)) <= 0) return;
    if (recv_line(cfd, s1, sizeof(s1)) <= 0) return;
    if (recv_line(cfd, s2, sizeof(s2)) <= 0) return;

    trim_newline(addr_line);
    trim_newline(s1);
    trim_newline(s2);

    print_datetime_with_addr(addr_line);

    if (are_anagrams(s1, s2)) {
        send_line(cfd, "Anagram: YES (strings are anagrams)");
    } else {
        send_line(cfd, "Anagram: NO (strings are not anagrams)");
    }
}

int main(void) {
    setvbuf(stdout, NULL, _IONBF, 0);
    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd < 0) {
        perror("socket");
        return 1;
    }

    int opt = 1;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(5002);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (bind(sfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(sfd);
        return 1;
    }

    if (listen(sfd, 16) < 0) {
        perror("listen");
        close(sfd);
        return 1;
    }

    printf("Q2 iterative server listening on 127.0.0.1:5002\n");
    printf("Serves one client at a time (iterative).\n");

    while (1) {
        struct sockaddr_in peer;
        socklen_t peer_len = sizeof(peer);
        int cfd = accept(sfd, (struct sockaddr *)&peer, &peer_len);
        if (cfd < 0) {
            if (errno == EINTR) continue;
            perror("accept");
            break;
        }

        handle_client(cfd);
        close(cfd);
    }

    close(sfd);
    return 0;
}
