/**
 * DNS CLIENT - Domain Name System Client using UNIX Domain Sockets
 * 
 * DESCRIPTION:
 * This program implements a DNS client that connects to a DNS server via UNIX
 * domain socket and queries domain names to get their corresponding IP addresses.
 * 
 * KEY FEATURES:
 * - Connects to DNS server using UNIX domain socket
 * - Interactive command-line interface for domain queries
 * - Displays query results (IP address or "not found" message)
 * - Supports "exit" command to terminate gracefully
 * 
 * USAGE:
 * ./dns_client [socket_path]
 * 
 * EXAMPLE:
 * ./dns_client ./dns_socket
 */

#include "dns_common.h"

#include <sys/un.h>

/**
 * usage() - Print usage information
 * @argv0: Program name (argv[0])
 */
static void usage(const char *argv0)
{
	fprintf(stderr, "Usage: %s [socket_path]\n", argv0);
	fprintf(stderr, "Default socket: %s\n", DNS_SOCK_PATH);
}

/**
 * main() - DNS Client main function
 * 
 * Process:
 * 1. Parse command-line arguments for socket path
 * 2. Create UNIX domain socket
 * 3. Connect to DNS server
 * 4. Display interactive prompt for domain queries
 * 5. Send queries to server and display results
 * 6. Continue until user types "exit"
 */
int main(int argc, char **argv)
{
	// Default socket path
	const char *sock_path = DNS_SOCK_PATH;
	
	// Parse command-line arguments
	if (argc >= 2)
		sock_path = argv[1];  // Use provided socket path
	if (argc > 2) {
		usage(argv[0]);
		return 1;
	}

	// Create a UNIX domain socket for communication
	int fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (fd < 0)
		dns_die("socket");  // Exit on socket creation failure

	// Set up socket address structure for connection
	struct sockaddr_un addr;
	memset(&addr, 0, sizeof(addr));            // Clear the structure
	addr.sun_family = AF_UNIX;                  // Use UNIX domain sockets
	strncpy(addr.sun_path, sock_path, sizeof(addr.sun_path) - 1);  // Set socket file path

	// Attempt to connect to the DNS server
	if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
		dns_die("connect");  // Exit if connection fails

	// Display welcome message to user
	printf("DNS client connected. Type a domain (or 'exit').\n\n");

	// Buffer to store user input (domain name)
	char line[MAX_DOMAIN];
	
	// Main loop: keep accepting queries until user types "exit"
	for (;;) {
		// Display prompt
		printf("domain> ");
		fflush(stdout);  // Ensure prompt is displayed immediately
		
		// Read user input from stdin
		if (!fgets(line, sizeof(line), stdin))
			break;  // End of input, exit loop
		
		// Remove trailing newline from input
		trim_newline_dns(line);
		
		// Skip empty input
		if (line[0] == '\0')
			continue;

		// Send domain query to server
		if (send_msg(fd, line) != 0)
			dns_die("send_msg");  // Exit on send failure

		// Buffer to store server's response
		char reply[MAX_DOMAIN + MAX_IP + 32];
		
		// Receive response from server
		int rr = recv_msg(fd, reply, sizeof(reply));
		if (rr == -2)
			break;  // Server closed connection
		if (rr != 0)
			dns_die("recv_msg");  // Exit on receive error

		// Display server's response to user
		printf("%s\n\n", reply);
		
		// Check if user requested exit
		if (strcasecmp(line, "exit") == 0)
			break;  // Exit the loop
	}

	// Close the socket connection
	close(fd);
	return 0;
}
