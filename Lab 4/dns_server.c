#include "dns_common.h"

#include <signal.h>
#include <sys/un.h>

typedef struct {
	char domain[MAX_DOMAIN];
	char ip[MAX_IP];
} entry_t;

static entry_t *g_entries = NULL;
static size_t g_entry_count = 0;
static size_t g_entry_cap = 0;

static void add_entry(const char *domain, const char *ip)
{
	if (g_entry_count == g_entry_cap) {
		size_t new_cap = g_entry_cap ? g_entry_cap * 2 : 16;
		entry_t *p = (entry_t *)realloc(g_entries, new_cap * sizeof(entry_t));
		if (!p)
			dns_die("realloc");
		g_entries = p;
		g_entry_cap = new_cap;
	}
	strncpy(g_entries[g_entry_count].domain, domain, MAX_DOMAIN - 1);
	g_entries[g_entry_count].domain[MAX_DOMAIN - 1] = '\0';
	strncpy(g_entries[g_entry_count].ip, ip, MAX_IP - 1);
	g_entries[g_entry_count].ip[MAX_IP - 1] = '\0';
	g_entry_count++;
}

static void load_db(const char *path)
{
	FILE *f = fopen(path, "r");
	if (!f)
		dns_die("fopen database.txt");

	char line[512];
	while (fgets(line, sizeof(line), f)) {
		trim_newline_dns(line);
		if (line[0] == '\0' || line[0] == '#')
			continue;
		char domain[MAX_DOMAIN];
		char ip[MAX_IP];
		if (sscanf(line, "%255s %63s", domain, ip) == 2) {
			add_entry(domain, ip);
		}
	}
	fclose(f);
}

static const char *lookup_ip(const char *domain)
{
	for (size_t i = 0; i < g_entry_count; i++) {
		if (strcasecmp(g_entries[i].domain, domain) == 0)
			return g_entries[i].ip;
	}
	return NULL;
}

static int g_listen_fd = -1;

static void cleanup_socket(void)
{
	unlink(DNS_SOCK_PATH);
}

static void on_sigint(int sig)
{
	(void)sig;
	if (g_listen_fd >= 0)
		close(g_listen_fd);
	cleanup_socket();
	_exit(0);
}

static void handle_client(int fd)
{
	for (;;) {
		char domain[MAX_DOMAIN];
		int rr = recv_msg(fd, domain, sizeof(domain));
		if (rr == -2)
			return; /* client closed */
		if (rr != 0) {
			(void)send_msg(fd, "ERROR: invalid request");
			return;
		}

		if (strcasecmp(domain, "exit") == 0) {
			(void)send_msg(fd, "bye");
			return;
		}

		const char *ip = lookup_ip(domain);
		char reply[MAX_DOMAIN + MAX_IP + 32];
		if (!ip) {
			snprintf(reply, sizeof(reply), "NOTFOUND %s", domain);
		} else {
			snprintf(reply, sizeof(reply), "OK %s %s", domain, ip);
		}
		(void)send_msg(fd, reply);
	}
}

static void usage(const char *argv0)
{
	fprintf(stderr, "Usage: %s [database.txt] [socket_path]\n", argv0);
	fprintf(stderr, "Default db: ./database.txt\n");
	fprintf(stderr, "Default socket: %s\n", DNS_SOCK_PATH);
}

int main(int argc, char **argv)
{
	const char *db_path = "./database.txt";
	const char *sock_path = DNS_SOCK_PATH;

	if (argc >= 2)
		db_path = argv[1];
	if (argc >= 3)
		sock_path = argv[2];
	if (argc > 3) {
		usage(argv[0]);
		return 1;
	}

	load_db(db_path);
	printf("[dns_server] Loaded %zu records from %s\n", g_entry_count, db_path);

	cleanup_socket();
	signal(SIGINT, on_sigint);

	g_listen_fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (g_listen_fd < 0)
		dns_die("socket");

	struct sockaddr_un addr;
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, sock_path, sizeof(addr.sun_path) - 1);

	if (bind(g_listen_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
		dns_die("bind");
	if (listen(g_listen_fd, 8) < 0)
		dns_die("listen");

	printf("[dns_server] Listening on UNIX socket %s\n", sock_path);

	for (;;) {
		int client_fd = accept(g_listen_fd, NULL, NULL);
		if (client_fd < 0) {
			if (errno == EINTR)
				continue;
			dns_die("accept");
		}
		handle_client(client_fd);
		close(client_fd);
	}
}
