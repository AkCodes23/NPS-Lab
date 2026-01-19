#include <arpa/inet.h>
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

static int send_all(int fd, const char *data, size_t len)
{
    size_t off = 0;
    while (off < len) {
        ssize_t n = send(fd, data + off, len - off, 0);
        if (n <= 0) {
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

static void flush_stdin_line(void)
{
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF) {
        ;
    }
}

static void print_server_reply(int sockfd)
{
    // Read until END or BYE
    char buf[MAX_LINE];
    while (1) {
        ssize_t n = recv(sockfd, buf, sizeof(buf) - 1, 0);
        if (n <= 0) {
            break;
        }
        buf[n] = '\0';
        fputs(buf, stdout);
        if (strstr(buf, "END\n") != NULL || strstr(buf, "BYE\n") != NULL) {
            break;
        }
        // If response is larger than one recv(), keep looping.
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

    while (1) {
        int option;
        printf("\nMenu:\n");
        printf("1. Add/Subtract two integers\n");
        printf("2. Find x in a linear equation (a*x + b = c)\n");
        printf("3. Multiply two matrices\n");
        printf("4. Exit\n");
        printf("Enter option: ");

        if (scanf("%d", &option) != 1) {
            printf("Invalid input. Exiting.\n");
            break;
        }
        flush_stdin_line();

        if (option < 1 || option > 4) {
            printf("Please choose 1-4.\n");
            continue;
        }

        if (send_fmt(sockfd, "OP %d\n", option) != 0) {
            perror("send");
            break;
        }

        if (option == 4) {
            print_server_reply(sockfd);
            break;
        }

        if (option == 1) {
            long long a, b;
            printf("Enter two integers (a b): ");
            if (scanf("%lld %lld", &a, &b) != 2) {
                printf("Invalid input.\n");
                flush_stdin_line();
                continue;
            }
            flush_stdin_line();

            if (send_fmt(sockfd, "A %lld %lld\n", a, b) != 0) {
                perror("send");
                break;
            }

            print_server_reply(sockfd);
            continue;
        }

        if (option == 2) {
            double a, b, c;
            printf("Solve a*x + b = c\n");
            printf("Enter a b c: ");
            if (scanf("%lf %lf %lf", &a, &b, &c) != 3) {
                printf("Invalid input.\n");
                flush_stdin_line();
                continue;
            }
            flush_stdin_line();

            if (send_fmt(sockfd, "LIN %.17g %.17g %.17g\n", a, b, c) != 0) {
                perror("send");
                break;
            }

            print_server_reply(sockfd);
            continue;
        }

        if (option == 3) {
            int r1, c1, r2, c2;
            printf("Matrix multiplication: (r1 x c1) * (r2 x c2)\n");
            printf("Enter r1 c1: ");
            if (scanf("%d %d", &r1, &c1) != 2) {
                printf("Invalid input.\n");
                flush_stdin_line();
                continue;
            }
            printf("Enter r2 c2: ");
            if (scanf("%d %d", &r2, &c2) != 2) {
                printf("Invalid input.\n");
                flush_stdin_line();
                continue;
            }
            flush_stdin_line();

            if (r1 <= 0 || c1 <= 0 || r2 <= 0 || c2 <= 0 || r1 > MAX_DIM || c1 > MAX_DIM || r2 > MAX_DIM || c2 > MAX_DIM) {
                printf("Dimensions must be between 1 and %d\n", MAX_DIM);
                continue;
            }

            long long m1[MAX_DIM][MAX_DIM];
            long long m2[MAX_DIM][MAX_DIM];
            memset(m1, 0, sizeof(m1));
            memset(m2, 0, sizeof(m2));

            printf("Enter elements of M1 (%d x %d), row-wise:\n", r1, c1);
            for (int i = 0; i < r1; i++) {
                for (int j = 0; j < c1; j++) {
                    printf("M1[%d][%d]: ", i, j);
                    if (scanf("%lld", &m1[i][j]) != 1) {
                        printf("Invalid input.\n");
                        flush_stdin_line();
                        goto matrix_done;
                    }
                }
            }

            printf("Enter elements of M2 (%d x %d), row-wise:\n", r2, c2);
            for (int i = 0; i < r2; i++) {
                for (int j = 0; j < c2; j++) {
                    printf("M2[%d][%d]: ", i, j);
                    if (scanf("%lld", &m2[i][j]) != 1) {
                        printf("Invalid input.\n");
                        flush_stdin_line();
                        goto matrix_done;
                    }
                }
            }
            flush_stdin_line();

            if (send_fmt(sockfd, "DIM %d %d %d %d\n", r1, c1, r2, c2) != 0) {
                perror("send");
                break;
            }
            if (send_fmt(sockfd, "M1\n") != 0) {
                perror("send");
                break;
            }
            for (int i = 0; i < r1; i++) {
                for (int j = 0; j < c1; j++) {
                    if (send_fmt(sockfd, "%lld%s", m1[i][j], (j + 1 == c1) ? "\n" : " ") != 0) {
                        perror("send");
                        break;
                    }
                }
            }
            if (send_fmt(sockfd, "M2\n") != 0) {
                perror("send");
                break;
            }
            for (int i = 0; i < r2; i++) {
                for (int j = 0; j < c2; j++) {
                    if (send_fmt(sockfd, "%lld%s", m2[i][j], (j + 1 == c2) ? "\n" : " ") != 0) {
                        perror("send");
                        break;
                    }
                }
            }

            print_server_reply(sockfd);
        }

    matrix_done:
        ;
    }

    close(sockfd);
    return 0;
}
