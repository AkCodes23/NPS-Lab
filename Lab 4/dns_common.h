#ifndef DNS_COMMON_H
#define DNS_COMMON_H

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>

#define DNS_SOCK_PATH "./dns_socket"
#define MAX_DOMAIN 256
#define MAX_IP 64

static inline void dns_die(const char *msg)
{
	perror(msg);
	exit(EXIT_FAILURE);
}

static inline int send_all_bytes(int fd, const void *buf, size_t len)
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

static inline int recv_all_bytes(int fd, void *buf, size_t len)
{
	uint8_t *p = (uint8_t *)buf;
	size_t off = 0;
	while (off < len) {
		ssize_t r = recv(fd, p + off, len - off, 0);
		if (r == 0)
			return -2; /* closed */
		if (r < 0) {
			if (errno == EINTR)
				continue;
			return -1;
		}
		off += (size_t)r;
	}
	return 0;
}

/* Simple length-prefixed message: uint32_t length (host order) + bytes (no NUL required). */
static inline int send_msg(int fd, const char *s)
{
	uint32_t len = (uint32_t)strlen(s);
	if (send_all_bytes(fd, &len, sizeof(len)) != 0)
		return -1;
	return send_all_bytes(fd, s, len);
}

static inline int recv_msg(int fd, char *out, size_t outsz)
{
	uint32_t len = 0;
	int rr = recv_all_bytes(fd, &len, sizeof(len));
	if (rr != 0)
		return rr;
	if (len >= outsz)
		return -3;
	if (len > 0) {
		rr = recv_all_bytes(fd, out, len);
		if (rr != 0)
			return rr;
	}
	out[len] = '\0';
	return 0;
}

static inline void trim_newline_dns(char *s)
{
	if (!s)
		return;
	size_t n = strlen(s);
	if (n > 0 && s[n - 1] == '\n')
		s[n - 1] = '\0';
}

#endif
