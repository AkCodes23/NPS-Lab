#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_CLIENTS 64
#define LINE_MAX 1024

static pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;
static int g_client_fds[MAX_CLIENTS];
static int g_client_count = 0;
static char g_client_addr_str[MAX_CLIENTS][128];
static char g_client_keyword[MAX_CLIENTS][64];

static volatile sig_atomic_t g_shutdown = 0;
static int g_listen_fd = -1;

static void trim_newline(char *s) {
    if (!s) return;
    size_t n = strlen(s);
    while (n > 0 && (s[n - 1] == '\n' || s[n - 1] == '\r')) {
        s[n - 1] = '\0';
        n--;
    }
}

static void normalize_keyword(char *keyword, size_t keyword_sz) {
    if (!keyword || keyword_sz == 0) return;
    if (strcasecmp(keyword, "Institute Of") == 0 || strcasecmp(keyword, "Institute of") == 0) {
        snprintf(keyword, keyword_sz, "%s", "Institute of");
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

static void broadcast_terminate_and_exit(void) {
    pthread_mutex_lock(&g_lock);
    for (int i = 0; i < g_client_count; i++) {
        if (g_client_fds[i] >= 0) {
            (void)send_line(g_client_fds[i], "terminate session");
        }
    }

    for (int i = 0; i < g_client_count; i++) {
        if (g_client_fds[i] >= 0) {
            shutdown(g_client_fds[i], SHUT_RDWR);
            close(g_client_fds[i]);
            g_client_fds[i] = -1;
        }
    }
    pthread_mutex_unlock(&g_lock);

    g_shutdown = 1;
    if (g_listen_fd >= 0) {
        close(g_listen_fd); // unblock accept
        g_listen_fd = -1;
    }
}

static int read_base_keyword(const char *path, char *out, size_t out_sz) {
    FILE *f = fopen(path, "r");
    if (!f) return -1;

    if (!fgets(out, (int)out_sz, f)) {
        fclose(f);
        return -1;
    }
    fclose(f);
    trim_newline(out);
    return 0;
}

static void maybe_print_phrase(void) {
    char base[128];
    if (read_base_keyword("Q1_base.txt", base, sizeof(base)) != 0) {
        fprintf(stderr, "Failed to read Q1_base.txt\n");
        return;
    }

    pthread_mutex_lock(&g_lock);
    if (g_client_count < 2) {
        pthread_mutex_unlock(&g_lock);
        return;
    }

    // Expect first two clients correspond to client1 and client2.
    const char *kw1 = g_client_keyword[0];
    const char *kw2 = g_client_keyword[1];
    const char *addr1 = g_client_addr_str[0];
    const char *addr2 = g_client_addr_str[1];
    pthread_mutex_unlock(&g_lock);

    printf("\n--- Result ---\n");
    printf("%s %s %s\n", base, kw1, kw2);
    printf("Client1 socket address (from client message): %s\n", addr1);
    printf("Client2 socket address (from client message): %s\n", addr2);
    printf("--------------\n\n");
    fflush(stdout);
}

struct client_ctx {
    int fd;
    struct sockaddr_in peer;
};

static void *client_thread(void *arg) {
    struct client_ctx *ctx = (struct client_ctx *)arg;
    int fd = ctx->fd;
    struct sockaddr_in peer = ctx->peer;
    free(ctx);

    char line[LINE_MAX];
    int rc = recv_line(fd, line, sizeof(line));
    if (rc <= 0) {
        close(fd);
        return NULL;
    }
    trim_newline(line);

    char keyword[64] = {0};
    char addr_str[128] = {0};

    // Expected: <keyword>|<ip>:<port>
    char *sep = strchr(line, '|');
    if (sep) {
        *sep = '\0';
        snprintf(keyword, sizeof(keyword), "%s", line);
        snprintf(addr_str, sizeof(addr_str), "%s", sep + 1);
    } else {
        snprintf(keyword, sizeof(keyword), "%s", line);
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &peer.sin_addr, ip, sizeof(ip));
        snprintf(addr_str, sizeof(addr_str), "%s:%u", ip, (unsigned)ntohs(peer.sin_port));
    }

    normalize_keyword(keyword, sizeof(keyword));

    int my_index = -1;

    pthread_mutex_lock(&g_lock);
    if (g_client_count < MAX_CLIENTS) {
        my_index = g_client_count;
        g_client_fds[g_client_count] = fd;
        snprintf(g_client_keyword[g_client_count], sizeof(g_client_keyword[g_client_count]), "%s", keyword);
        snprintf(g_client_addr_str[g_client_count], sizeof(g_client_addr_str[g_client_count]), "%s", addr_str);
        g_client_count++;
    }
    int count_now = g_client_count;
    pthread_mutex_unlock(&g_lock);

    if (my_index < 0) {
        send_line(fd, "server full");
        close(fd);
        return NULL;
    }

    if (count_now > 2) {
        broadcast_terminate_and_exit();
        return NULL;
    }

    if (count_now == 2) {
        maybe_print_phrase();
    }

    // Keep connection open until server shutdown or peer closes.
    while (!g_shutdown) {
        char buf[64];
        ssize_t n = recv(fd, buf, sizeof(buf), 0);
        if (n <= 0) break;
    }

    close(fd);
    return NULL;
}

int main(void) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        g_client_fds[i] = -1;
    }

    g_listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (g_listen_fd < 0) {
        perror("socket");
        return 1;
    }

    int opt = 1;
    setsockopt(g_listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(5001);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (bind(g_listen_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(g_listen_fd);
        return 1;
    }

    if (listen(g_listen_fd, 16) < 0) {
        perror("listen");
        close(g_listen_fd);
        return 1;
    }

    printf("Q1 concurrent server listening on 127.0.0.1:5001\n");
    printf("Base text file: Q1_base.txt\n");
    printf("Waiting for clients... (3rd client triggers termination)\n");

    while (!g_shutdown) {
        struct sockaddr_in peer;
        socklen_t peer_len = sizeof(peer);
        int cfd = accept(g_listen_fd, (struct sockaddr *)&peer, &peer_len);
        if (cfd < 0) {
            if (errno == EINTR) continue;
            if (g_shutdown) break;
            perror("accept");
            break;
        }

        struct client_ctx *ctx = (struct client_ctx *)malloc(sizeof(*ctx));
        if (!ctx) {
            close(cfd);
            continue;
        }
        ctx->fd = cfd;
        ctx->peer = peer;

        pthread_t tid;
        if (pthread_create(&tid, NULL, client_thread, ctx) != 0) {
            close(cfd);
            free(ctx);
            continue;
        }
        pthread_detach(tid);
    }

    return 0;
}
