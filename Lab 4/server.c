#include "common.h"

#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

typedef struct {
	const char *regno;
	const char *name;
	const char *address;
	const char *dept;
	const char *semester;
	const char *section;
	const char *courses;
} student_t;

typedef struct {
	const char *subject;
	int marks;
} marks_t;

static const student_t g_students[] = {
	{
		.regno = "23CS001",
		.name = "Asha",
		.address = "12, MG Road, Bengaluru",
		.dept = "CSE",
		.semester = "4",
		.section = "A",
		.courses = "CS201, CS202, MA201",
	},
	{
		.regno = "23EC014",
		.name = "Rahul",
		.address = "44, Lake View, Chennai",
		.dept = "ECE",
		.semester = "3",
		.section = "B",
		.courses = "EC210, EC211, MA201",
	},
};

static const marks_t g_marks[] = {
	{.subject = "CS201", .marks = 88},
	{.subject = "CS202", .marks = 79},
	{.subject = "MA201", .marks = 91},
	{.subject = "EC210", .marks = 84},
	{.subject = "EC211", .marks = 77},
};

static const student_t *find_by_regno(const char *regno)
{
	for (size_t i = 0; i < sizeof(g_students) / sizeof(g_students[0]); i++) {
		if (strncmp(g_students[i].regno, regno, MAX_REGNO) == 0)
			return &g_students[i];
	}
	return NULL;
}

static const student_t *find_by_name(const char *name)
{
	for (size_t i = 0; i < sizeof(g_students) / sizeof(g_students[0]); i++) {
		if (strcasecmp(g_students[i].name, name) == 0)
			return &g_students[i];
	}
	return NULL;
}

static const marks_t *find_marks(const char *subject)
{
	for (size_t i = 0; i < sizeof(g_marks) / sizeof(g_marks[0]); i++) {
		if (strcasecmp(g_marks[i].subject, subject) == 0)
			return &g_marks[i];
	}
	return NULL;
}

typedef struct {
	int p2c[2]; /* parent -> child */
	int c2p[2]; /* child -> parent */
	pid_t pid;
	option_t role;
} worker_t;

static void close_fd(int *fd)
{
	if (*fd >= 0)
		close(*fd);
	*fd = -1;
}

static void worker_loop(option_t role, int read_fd, int write_fd)
{
	for (;;) {
		request_t req;
		ssize_t r = read(read_fd, &req, sizeof(req));
		if (r == 0)
			_exit(0);
		if (r < 0) {
			if (errno == EINTR)
				continue;
			_exit(2);
		}
		if ((size_t)r != sizeof(req))
			continue;

		response_t resp;
		memset(&resp, 0, sizeof(resp));
		resp.magic = APP_MAGIC;
		resp.status = 0;
		resp.child_pid = (int32_t)getpid();

		if (req.magic != APP_MAGIC) {
			resp.status = 1;
			snprintf(resp.message, sizeof(resp.message), "Invalid request magic");
		} else if ((option_t)req.option != role) {
			resp.status = 2;
			snprintf(resp.message, sizeof(resp.message), "Wrong worker role");
		} else if (role == OPT_REGNO) {
			const student_t *s = find_by_regno(req.regno);
			if (!s) {
				resp.status = 3;
				snprintf(resp.message, sizeof(resp.message),
					"Registration '%s' not found", req.regno);
			} else {
				snprintf(resp.message, sizeof(resp.message),
					"Name: %s\nAddress: %s\nChild PID: %d", s->name,
					s->address, (int)getpid());
			}
		} else if (role == OPT_NAME) {
			const student_t *s = find_by_name(req.name);
			if (!s) {
				resp.status = 4;
				snprintf(resp.message, sizeof(resp.message),
					"Name '%s' not found", req.name);
			} else {
				snprintf(resp.message, sizeof(resp.message),
					"Dept: %s\nSemester: %s\nSection: %s\nCourses: %s\nChild PID: %d",
					s->dept, s->semester, s->section, s->courses, (int)getpid());
			}
		} else if (role == OPT_SUBJECT) {
			const marks_t *m = find_marks(req.subject);
			if (!m) {
				resp.status = 5;
				snprintf(resp.message, sizeof(resp.message),
					"Subject '%s' not found", req.subject);
			} else {
				snprintf(resp.message, sizeof(resp.message),
					"Subject: %s\nMarks: %d\nChild PID: %d", m->subject,
					m->marks, (int)getpid());
			}
		} else {
			resp.status = 6;
			snprintf(resp.message, sizeof(resp.message), "Unknown option");
		}

		(void)write(write_fd, &resp, sizeof(resp));
	}
}

static void spawn_worker(worker_t *w, option_t role)
{
	memset(w, 0, sizeof(*w));
	w->p2c[0] = w->p2c[1] = -1;
	w->c2p[0] = w->c2p[1] = -1;
	w->role = role;

	if (pipe(w->p2c) < 0)
		die("pipe p2c");
	if (pipe(w->c2p) < 0)
		die("pipe c2p");

	pid_t pid = fork();
	if (pid < 0)
		die("fork");
	if (pid == 0) {
		/* child */
		close_fd(&w->p2c[1]);
		close_fd(&w->c2p[0]);
		worker_loop(role, w->p2c[0], w->c2p[1]);
		_exit(0);
	}

	w->pid = pid;
	/* parent */
	close_fd(&w->p2c[0]);
	close_fd(&w->c2p[1]);
}

static int route_to_worker(worker_t workers[3], const request_t *req, response_t *out)
{
	int idx = -1;
	if (req->option == OPT_REGNO)
		idx = 0;
	else if (req->option == OPT_NAME)
		idx = 1;
	else if (req->option == OPT_SUBJECT)
		idx = 2;
	else
		return -1;

	if (write(workers[idx].p2c[1], req, sizeof(*req)) != (ssize_t)sizeof(*req))
		return -2;
	ssize_t r = read(workers[idx].c2p[0], out, sizeof(*out));
	if (r != (ssize_t)sizeof(*out))
		return -3;
	return 0;
}

static void usage(const char *argv0)
{
	fprintf(stderr, "Usage: %s <tcp|udp> <port>\n", argv0);
}

static int run_tcp(uint16_t port, worker_t workers[3])
{
	int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_fd < 0)
		die("socket");

	int opt = 1;
	setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);

	if (bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
		die("bind");
	if (listen(listen_fd, 16) < 0)
		die("listen");

	printf("[server] TCP listening on %u\n", port);

	for (;;) {
		struct sockaddr_in cli;
		socklen_t clilen = sizeof(cli);
		int conn_fd = accept(listen_fd, (struct sockaddr *)&cli, &clilen);
		if (conn_fd < 0) {
			if (errno == EINTR)
				continue;
			die("accept");
		}

		request_t req;
		response_t resp;
		memset(&resp, 0, sizeof(resp));
		resp.magic = APP_MAGIC;
		resp.status = 99;
		snprintf(resp.message, sizeof(resp.message), "Server error");

		int rr = recv_all(conn_fd, &req, sizeof(req));
		if (rr == 0) {
			if (req.magic != APP_MAGIC) {
				resp.status = 1;
				snprintf(resp.message, sizeof(resp.message), "Invalid request");
			} else {
				if (route_to_worker(workers, &req, &resp) != 0) {
					resp.status = 2;
					snprintf(resp.message, sizeof(resp.message), "Routing failed");
				}
			}
		} else {
			resp.status = 3;
			snprintf(resp.message, sizeof(resp.message), "Failed to read request");
		}

		(void)send_all(conn_fd, &resp, sizeof(resp));
		close(conn_fd);
	}

	return 0;
}

static int run_udp(uint16_t port, worker_t workers[3])
{
	int fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0)
		die("socket");

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);

	if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
		die("bind");

	printf("[server] UDP listening on %u\n", port);

	for (;;) {
		request_t req;
		struct sockaddr_in peer;
		socklen_t peerlen = sizeof(peer);
		ssize_t n = recvfrom(fd, &req, sizeof(req), 0, (struct sockaddr *)&peer, &peerlen);
		if (n < 0) {
			if (errno == EINTR)
				continue;
			die("recvfrom");
		}

		response_t resp;
		memset(&resp, 0, sizeof(resp));
		resp.magic = APP_MAGIC;
		if ((size_t)n != sizeof(req) || req.magic != APP_MAGIC) {
			resp.status = 1;
			snprintf(resp.message, sizeof(resp.message), "Invalid request");
		} else {
			if (route_to_worker(workers, &req, &resp) != 0) {
				resp.status = 2;
				snprintf(resp.message, sizeof(resp.message), "Routing failed");
			}
		}

		(void)sendto(fd, &resp, sizeof(resp), 0, (struct sockaddr *)&peer, peerlen);
	}

	return 0;
}

int main(int argc, char **argv)
{
	if (argc != 3) {
		usage(argv[0]);
		return 1;
	}

	const char *mode = argv[1];
	long port_l = strtol(argv[2], NULL, 10);
	if (port_l <= 0 || port_l > 65535) {
		fprintf(stderr, "Invalid port\n");
		return 1;
	}
	uint16_t port = (uint16_t)port_l;

	/* Keep children alive even if parent ignores SIGCHLD (avoid zombies if they exit). */
	signal(SIGCHLD, SIG_IGN);

	worker_t workers[3];
	spawn_worker(&workers[0], OPT_REGNO);
	spawn_worker(&workers[1], OPT_NAME);
	spawn_worker(&workers[2], OPT_SUBJECT);

	printf("[server] Workers: regno=%d, name=%d, subject=%d\n", (int)workers[0].pid,
		(int)workers[1].pid, (int)workers[2].pid);

	if (strcmp(mode, "tcp") == 0)
		return run_tcp(port, workers);
	if (strcmp(mode, "udp") == 0)
		return run_udp(port, workers);

	usage(argv[0]);
	return 1;
}
