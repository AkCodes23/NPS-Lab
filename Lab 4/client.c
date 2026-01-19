#include "common.h"

#include <ctype.h>

static void usage(const char *argv0)
{
	fprintf(stderr, "Usage: %s <tcp|udp> <server_ip> <port>\n", argv0);
}

static int prompt_option(void)
{
	char line[32];
	for (;;) {
		printf("Choose option:\n");
		printf("  1. Registration Number\n");
		printf("  2. Name of the Student\n");
		printf("  3. Subject Code\n");
		printf("Enter option (1-3): ");
		fflush(stdout);

		if (!fgets(line, sizeof(line), stdin))
			return -1;
		trim_newline(line);
		int opt = atoi(line);
		if (opt >= 1 && opt <= 3)
			return opt;
		printf("Invalid option. Try again.\n\n");
	}
}

static void prompt_string(const char *label, char *out, size_t outsz)
{
	for (;;) {
		printf("Enter %s: ", label);
		fflush(stdout);
		if (!fgets(out, outsz, stdin)) {
			out[0] = '\0';
			return;
		}
		trim_newline(out);
		if (out[0] != '\0')
			return;
		printf("Value cannot be empty.\n");
	}
}

static int run_tcp(const char *ip, uint16_t port, const request_t *req, response_t *resp)
{
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0)
		die("socket");

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	if (inet_pton(AF_INET, ip, &addr.sin_addr) != 1) {
		fprintf(stderr, "Invalid IP address\n");
		close(fd);
		return 1;
	}

	if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
		die("connect");

	if (send_all(fd, req, sizeof(*req)) != 0)
		die("send");
	if (recv_all(fd, resp, sizeof(*resp)) != 0)
		die("recv");

	close(fd);
	return 0;
}

static int run_udp(const char *ip, uint16_t port, const request_t *req, response_t *resp)
{
	int fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0)
		die("socket");

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	if (inet_pton(AF_INET, ip, &addr.sin_addr) != 1) {
		fprintf(stderr, "Invalid IP address\n");
		close(fd);
		return 1;
	}

	ssize_t s = sendto(fd, req, sizeof(*req), 0, (struct sockaddr *)&addr, sizeof(addr));
	if (s < 0)
		die("sendto");

	struct sockaddr_in peer;
	socklen_t peerlen = sizeof(peer);
	ssize_t r = recvfrom(fd, resp, sizeof(*resp), 0, (struct sockaddr *)&peer, &peerlen);
	if (r < 0)
		die("recvfrom");

	close(fd);
	return 0;
}

int main(int argc, char **argv)
{
	if (argc != 4) {
		usage(argv[0]);
		return 1;
	}

	const char *mode = argv[1];
	const char *ip = argv[2];
	long port_l = strtol(argv[3], NULL, 10);
	if (port_l <= 0 || port_l > 65535) {
		fprintf(stderr, "Invalid port\n");
		return 1;
	}
	uint16_t port = (uint16_t)port_l;

	int opt = prompt_option();
	if (opt < 0)
		return 1;

	request_t req;
	memset(&req, 0, sizeof(req));
	req.magic = APP_MAGIC;
	req.option = (uint32_t)opt;

	if (opt == OPT_REGNO) {
		prompt_string("Registration Number", req.regno, sizeof(req.regno));
	} else if (opt == OPT_NAME) {
		prompt_string("Name of the Student", req.name, sizeof(req.name));
	} else if (opt == OPT_SUBJECT) {
		prompt_string("Subject Code", req.subject, sizeof(req.subject));
	}

	response_t resp;
	memset(&resp, 0, sizeof(resp));

	int rc;
	if (strcmp(mode, "tcp") == 0) {
		rc = run_tcp(ip, port, &req, &resp);
	} else if (strcmp(mode, "udp") == 0) {
		rc = run_udp(ip, port, &req, &resp);
	} else {
		usage(argv[0]);
		return 1;
	}

	if (rc != 0)
		return rc;

	if (resp.magic != APP_MAGIC) {
		fprintf(stderr, "Invalid response from server\n");
		return 1;
	}

	printf("\n--- Server Reply ---\n");
	printf("Status: %d\n", (int)resp.status);
	printf("Worker PID: %d\n", (int)resp.child_pid);
	printf("Details:\n%s\n", resp.message);
	return 0;
}
