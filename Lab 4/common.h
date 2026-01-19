#ifndef COMMON_H
#define COMMON_H

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#ifdef __cplusplus
extern "C" {
#endif

#define APP_MAGIC 0x4C423431u /* "LB41" */

#define MAX_REGNO 32
#define MAX_NAME 64
#define MAX_SUBJECT 16
#define MAX_MESSAGE 512

typedef enum {
	OPT_REGNO = 1,
	OPT_NAME = 2,
	OPT_SUBJECT = 3
} option_t;

typedef struct {
	uint32_t magic; /* APP_MAGIC */
	uint32_t option; /* option_t */
	char regno[MAX_REGNO];
	char name[MAX_NAME];
	char subject[MAX_SUBJECT];
} request_t;

typedef struct {
	uint32_t magic; /* APP_MAGIC */
	int32_t status; /* 0 ok, nonzero error */
	int32_t child_pid;
	char message[MAX_MESSAGE];
} response_t;

static inline void die(const char *msg)
{
	perror(msg);
	exit(EXIT_FAILURE);
}

static inline void trim_newline(char *s)
{
	if (!s)
		return;
	size_t n = strlen(s);
	if (n > 0 && s[n - 1] == '\n')
		s[n - 1] = '\0';
}

static inline int send_all(int fd, const void *buf, size_t len)
{
	const uint8_t *p = (const uint8_t *)buf;
	size_t off = 0;
	while (off < len) {
		ssize_t w = send(fd, p + off, len - off, 0);
		if (w < 0) {
			if (errno == EINTR)
				continue;
			return -1;
		}
		off += (size_t)w;
	}
	return 0;
}

static inline int recv_all(int fd, void *buf, size_t len)
{
	uint8_t *p = (uint8_t *)buf;
	size_t off = 0;
	while (off < len) {
		ssize_t r = recv(fd, p + off, len - off, 0);
		if (r == 0)
			return -2; /* peer closed */
		if (r < 0) {
			if (errno == EINTR)
				continue;
			return -1;
		}
		off += (size_t)r;
	}
	return 0;
}

#ifdef __cplusplus
}
#endif

#endif
