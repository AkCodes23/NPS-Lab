#include "dns_common.h"

#include <sys/un.h>

static void usage(const char *argv0)
{
	fprintf(stderr, "Usage: %s [socket_path]\n", argv0);
	fprintf(stderr, "Default socket: %s\n", DNS_SOCK_PATH);
}

int main(int argc, char **argv)
{
	const char *sock_path = DNS_SOCK_PATH;
	if (argc >= 2)
		sock_path = argv[1];
	if (argc > 2) {
		usage(argv[0]);
		return 1;
	}

	int fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (fd < 0)
		dns_die("socket");

	struct sockaddr_un addr;
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, sock_path, sizeof(addr.sun_path) - 1);

	if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
		dns_die("connect");

	printf("DNS client connected. Type a domain (or 'exit').\n\n");

	char line[MAX_DOMAIN];
	for (;;) {
		printf("domain> ");
		fflush(stdout);
		if (!fgets(line, sizeof(line), stdin))
			break;
		trim_newline_dns(line);
		if (line[0] == '\0')
			continue;

		if (send_msg(fd, line) != 0)
			dns_die("send_msg");

		char reply[MAX_DOMAIN + MAX_IP + 32];
		int rr = recv_msg(fd, reply, sizeof(reply));
		if (rr == -2)
			break;
		if (rr != 0)
			dns_die("recv_msg");

		printf("%s\n\n", reply);
		if (strcasecmp(line, "exit") == 0)
			break;
	}

	close(fd);
	return 0;
}
