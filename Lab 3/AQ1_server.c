#include <arpa/inet.h>
#include <ctype.h>
#include <netinet/in.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define DEFAULT_PORT 6060
#define MAX_LINE 4096
#define MAX_DIM 10

static ssize_t read_line(int fd, char *buf, size_t maxlen)
{
    size_t i = 0;
    while (i + 1 < maxlen) {
        char c;
        ssize_t n = recv(fd, &c, 1, 0);
        if (n == 0) {
            break;
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

static int send_all(int fd, const char *data, size_t len)
{
    size_t off = 0;
    while (off < len) {
        ssize_t n = send(fd, data + off, len - off, 0);
        if (n < 0) {
            return -1;
        }
        if (n == 0) {
            return -1;
        }
        off += (size_t)n;
    }
    return 0;
}

static int send_fmt(int fd, const char *fmt, ...)
{
    char buf[MAX_LINE];
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

    if (n < 0) {
        return -1;
    }
    size_t len = (size_t)n;
    if (len >= sizeof(buf)) {
        len = sizeof(buf) - 1;
    }
    return send_all(fd, buf, len);
}

static int parse_op(const char *line, int *op_out)
{
    int op = 0;
    if (sscanf(line, "OP %d", &op) == 1) {
        *op_out = op;
        return 0;
    }
    return -1;
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

    printf("AQ1 server listening on port %d...\n", port);

    struct sockaddr_in clientaddr;
    socklen_t clientlen = sizeof(clientaddr);
    int clientfd = accept(sockfd, (struct sockaddr *)&clientaddr, &clientlen);
    if (clientfd < 0) {
        perror("accept");
        close(sockfd);
        return 1;
    }

    printf("Client connected from %s:%d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

    char line[MAX_LINE];
    while (1) {
        ssize_t n = read_line(clientfd, line, sizeof(line));
        if (n == 0) {
            break; // client closed
        }
        if (n < 0) {
            perror("recv");
            break;
        }

        int op = 0;
        if (parse_op(line, &op) != 0) {
            (void)send_fmt(clientfd, "ERR Invalid request\nEND\n");
            continue;
        }

        if (op == 4) {
            (void)send_fmt(clientfd, "BYE\n");
            break;
        }

        if (op == 1) {
            // Expect: A a b
            if (read_line(clientfd, line, sizeof(line)) <= 0) {
                (void)send_fmt(clientfd, "ERR Missing operands\nEND\n");
                continue;
            }
            long long a, b;
            if (sscanf(line, "A %lld %lld", &a, &b) != 2) {
                (void)send_fmt(clientfd, "ERR Invalid operands\nEND\n");
                continue;
            }

            (void)send_fmt(clientfd, "PID %d\nADD %lld\nSUB %lld\nEND\n", (int)getpid(), a + b, a - b);
            continue;
        }

        if (op == 2) {
            // Linear equation: a*x + b = c
            // Expect: LIN a b c
            if (read_line(clientfd, line, sizeof(line)) <= 0) {
                (void)send_fmt(clientfd, "ERR Missing coefficients\nEND\n");
                continue;
            }

            double a, b, c;
            if (sscanf(line, "LIN %lf %lf %lf", &a, &b, &c) != 3) {
                (void)send_fmt(clientfd, "ERR Invalid coefficients\nEND\n");
                continue;
            }

            if (a == 0.0) {
                if (b == c) {
                    (void)send_fmt(clientfd, "PID %d\nINF Infinite solutions\nEND\n", (int)getpid());
                } else {
                    (void)send_fmt(clientfd, "PID %d\nNONE No solution\nEND\n", (int)getpid());
                }
            } else {
                double x = (c - b) / a;
                (void)send_fmt(clientfd, "PID %d\nX %.6f\nEND\n", (int)getpid(), x);
            }
            continue;
        }

        if (op == 3) {
            // Expect: DIM r1 c1 r2 c2 then M1 lines then M2 lines
            if (read_line(clientfd, line, sizeof(line)) <= 0) {
                (void)send_fmt(clientfd, "ERR Missing dimensions\nEND\n");
                continue;
            }

            int r1, c1, r2, c2;
            if (sscanf(line, "DIM %d %d %d %d", &r1, &c1, &r2, &c2) != 4) {
                (void)send_fmt(clientfd, "ERR Invalid dimensions\nEND\n");
                continue;
            }

            if (r1 <= 0 || c1 <= 0 || r2 <= 0 || c2 <= 0 || r1 > MAX_DIM || c1 > MAX_DIM || r2 > MAX_DIM || c2 > MAX_DIM) {
                (void)send_fmt(clientfd, "ERR Dimensions must be between 1 and %d\nEND\n", MAX_DIM);
                continue;
            }

            if (c1 != r2) {
                (void)send_fmt(clientfd, "PID %d\nERR Incompatible matrices (c1 must equal r2)\nEND\n", (int)getpid());
                continue;
            }

            // Read marker M1
            if (read_line(clientfd, line, sizeof(line)) <= 0 || strcmp(line, "M1") != 0) {
                (void)send_fmt(clientfd, "ERR Expected M1\nEND\n");
                continue;
            }

            long long m1[MAX_DIM][MAX_DIM];
            long long m2[MAX_DIM][MAX_DIM];
            memset(m1, 0, sizeof(m1));
            memset(m2, 0, sizeof(m2));

            for (int i = 0; i < r1; i++) {
                if (read_line(clientfd, line, sizeof(line)) <= 0) {
                    (void)send_fmt(clientfd, "ERR Missing matrix M1 row\nEND\n");
                    goto next_loop;
                }
                char *p = line;
                for (int j = 0; j < c1; j++) {
                    while (*p == ' ') p++;
                    char *endp = NULL;
                    long long v = strtoll(p, &endp, 10);
                    if (endp == p) {
                        (void)send_fmt(clientfd, "ERR Invalid M1 data\nEND\n");
                        goto next_loop;
                    }
                    m1[i][j] = v;
                    p = endp;
                }
            }

            // Read marker M2
            if (read_line(clientfd, line, sizeof(line)) <= 0 || strcmp(line, "M2") != 0) {
                (void)send_fmt(clientfd, "ERR Expected M2\nEND\n");
                continue;
            }

            for (int i = 0; i < r2; i++) {
                if (read_line(clientfd, line, sizeof(line)) <= 0) {
                    (void)send_fmt(clientfd, "ERR Missing matrix M2 row\nEND\n");
                    goto next_loop;
                }
                char *p = line;
                for (int j = 0; j < c2; j++) {
                    while (*p == ' ') p++;
                    char *endp = NULL;
                    long long v = strtoll(p, &endp, 10);
                    if (endp == p) {
                        (void)send_fmt(clientfd, "ERR Invalid M2 data\nEND\n");
                        goto next_loop;
                    }
                    m2[i][j] = v;
                    p = endp;
                }
            }

            // Multiply
            long long prod[MAX_DIM][MAX_DIM];
            memset(prod, 0, sizeof(prod));

            for (int i = 0; i < r1; i++) {
                for (int j = 0; j < c2; j++) {
                    long long sum = 0;
                    for (int k = 0; k < c1; k++) {
                        sum += m1[i][k] * m2[k][j];
                    }
                    prod[i][j] = sum;
                }
            }

            (void)send_fmt(clientfd, "PID %d\nDIM %d %d\nMAT\n", (int)getpid(), r1, c2);
            for (int i = 0; i < r1; i++) {
                for (int j = 0; j < c2; j++) {
                    (void)send_fmt(clientfd, "%lld%s", prod[i][j], (j + 1 == c2) ? "\n" : " ");
                }
            }
            (void)send_fmt(clientfd, "END\n");
            continue;
        }

        (void)send_fmt(clientfd, "PID %d\nERR Unknown option\nEND\n", (int)getpid());

    next_loop:
        ;
    }

    close(clientfd);
    close(sockfd);
    return 0;
}
