/**
 * DNS SERVER - Domain Name System Server using UNIX Domain Sockets
 * 
 * DESCRIPTION:
 * This program implements a simple DNS server that resolves domain names to IP addresses.
 * It reads a database file (database.txt) containing domain-to-IP mappings and serves
 * DNS resolution requests from clients over UNIX domain sockets.
 * 
 * KEY FEATURES:
 * - Loads domain database from a text file
 * - Accepts client connections via UNIX domain socket
 * - Performs domain name lookups
 * - Returns IP address for valid domains, "NOTFOUND" for invalid ones
 * - Handles multiple clients sequentially
 * - Supports exit command for client termination
 * 
 * USAGE:
 * ./dns_server [database.txt] [socket_path]
 * 
 * EXAMPLE:
 * ./dns_server ./database.txt ./dns_socket
 */

#include "dns_common.h"

#include <signal.h>
#include <sys/un.h>

// Structure to store a single domain-to-IP mapping
typedef struct {
	char domain[MAX_DOMAIN];  // Domain name (e.g., "example.com")
	char ip[MAX_IP];           // IP address (e.g., "93.184.216.34")
} entry_t;

// Global database of domain entries
static entry_t *g_entries = NULL;
// Number of entries currently in the database
static size_t g_entry_count = 0;
// Capacity of the allocated entries array
static size_t g_entry_cap = 0;

/**
 * add_entry() - Add a new domain-IP mapping to the database
 * @domain: The domain name to add
 * @ip: The corresponding IP address
 * 
 * This function adds a new entry to the global database. It handles dynamic
 * memory allocation by doubling the capacity when needed (standard reallocation).
 */
static void add_entry(const char *domain, const char *ip)
{
	// Check if we need more space
	if (g_entry_count == g_entry_cap) {
		// Calculate new capacity (start with 16, double when full)
		size_t new_cap = g_entry_cap ? g_entry_cap * 2 : 16;
		
		// Allocate new memory block
		entry_t *p = (entry_t *)realloc(g_entries, new_cap * sizeof(entry_t));
		if (!p)
			dns_die("realloc");  // Exit on allocation failure
		
		g_entries = p;         // Update pointer to new memory
		g_entry_cap = new_cap; // Update capacity
	}
	
	// Copy domain name to database (truncate if too long)
	strncpy(g_entries[g_entry_count].domain, domain, MAX_DOMAIN - 1);
	g_entries[g_entry_count].domain[MAX_DOMAIN - 1] = '\0';  // Ensure null termination
	
	// Copy IP address to database (truncate if too long)
	strncpy(g_entries[g_entry_count].ip, ip, MAX_IP - 1);
	g_entries[g_entry_count].ip[MAX_IP - 1] = '\0';  // Ensure null termination
	
	// Increment counter for next entry
	g_entry_count++;
}

/**
 * load_db() - Load domain database from a file
 * @path: Path to the database file
 * 
 * Reads a text file with one domain-IP pair per line.
 * Lines starting with '#' are treated as comments and skipped.
 * Format: domain_name ip_address
 */
static void load_db(const char *path)
{
	// Open database file for reading
	FILE *f = fopen(path, "r");
	if (!f)
		dns_die("fopen database.txt");  // Exit if file cannot be opened

	// Read file line by line
	char line[512];
	while (fgets(line, sizeof(line), f)) {
		// Remove trailing newline character
		trim_newline_dns(line);
		
		// Skip empty lines and comments
		if (line[0] == '\0' || line[0] == '#')
			continue;
		
		// Parse domain and IP from the line
		char domain[MAX_DOMAIN];
		char ip[MAX_IP];
		if (sscanf(line, "%255s %63s", domain, ip) == 2) {
			// Successfully parsed both domain and IP
			add_entry(domain, ip);
		}
	}
	// Close file after reading
	fclose(f);
}

/**
 * lookup_ip() - Search for IP address of a given domain
 * @domain: Domain name to look up
 * 
 * Returns: Pointer to IP string if found, NULL if not found
 * 
 * Performs linear search through the database using case-insensitive comparison.
 * This is sufficient for small databases but would need optimization for large ones.
 */
static const char *lookup_ip(const char *domain)
{
	// Iterate through all entries in the database
	for (size_t i = 0; i < g_entry_count; i++) {
		// Case-insensitive comparison of domain names
		if (strcasecmp(g_entries[i].domain, domain) == 0)
			return g_entries[i].ip;  // Found match, return IP
	}
	return NULL;  // Domain not found
}

// Global file descriptor for the listening socket
static int g_listen_fd = -1;

/**
 * cleanup_socket() - Remove the UNIX socket file
 * 
 * Called during shutdown to clean up the socket file that was created by bind().
 */
static void cleanup_socket(void)
{
	unlink(DNS_SOCK_PATH);  // Remove socket file from filesystem
}

/**
 * on_sigint() - Signal handler for SIGINT (Ctrl+C)
 * @sig: Signal number (not used)
 * 
 * Handles graceful shutdown when user presses Ctrl+C.
 * Closes the listening socket and removes the socket file.
 */
static void on_sigint(int sig)
{
	(void)sig;  // Mark sig as used to avoid compiler warning
	
	// Close the listening socket if it's open
	if (g_listen_fd >= 0)
		close(g_listen_fd);
	
	// Clean up socket file
	cleanup_socket();
	
	// Exit the program
	_exit(0);
}

/**
 * handle_client() - Process requests from a connected client
 * @fd: File descriptor of the connected client socket
 * 
 * Runs a loop to receive domain queries from the client and send back results.
 * Returns when client disconnects or sends "exit" command.
 */
static void handle_client(int fd)
{
	// Loop to handle multiple requests from same client
	for (;;) {
		// Buffer to store domain name from client
		char domain[MAX_DOMAIN];
		
		// Receive domain name from client
		int rr = recv_msg(fd, domain, sizeof(domain));
		if (rr == -2)
			return;  // Client closed connection
		if (rr != 0) {
			// Error receiving message
			(void)send_msg(fd, "ERROR: invalid request");
			return;
		}

		// Check if client wants to exit
		if (strcasecmp(domain, "exit") == 0) {
			(void)send_msg(fd, "bye");  // Send goodbye message
			return;  // Close connection
		}

		// Look up the IP address for the requested domain
		const char *ip = lookup_ip(domain);
		
		// Prepare response message
		char reply[MAX_DOMAIN + MAX_IP + 32];
		if (!ip) {
			// Domain not found in database
			snprintf(reply, sizeof(reply), "NOTFOUND %s", domain);
		} else {
			// Domain found, return domain and IP
			snprintf(reply, sizeof(reply), "OK %s %s", domain, ip);
		}
		
		// Send response back to client
		(void)send_msg(fd, reply);
	}
}

/**
 * usage() - Print usage information
 * @argv0: Program name (argv[0])
 */
static void usage(const char *argv0)
{
	fprintf(stderr, "Usage: %s [database.txt] [socket_path]\n", argv0);
	fprintf(stderr, "Default db: ./database.txt\n");
	fprintf(stderr, "Default socket: %s\n", DNS_SOCK_PATH);
}

/**
 * main() - DNS Server main function
 * 
 * Process:
 * 1. Parse command-line arguments for database path and socket path
 * 2. Load domain database from file
 * 3. Create UNIX domain socket for listening
 * 4. Accept client connections in an infinite loop
 * 5. Handle each client's DNS queries until they disconnect
 */
int main(int argc, char **argv)
{
	// Default paths
	const char *db_path = "./database.txt";
	const char *sock_path = DNS_SOCK_PATH;

	// Parse command-line arguments
	if (argc >= 2)
		db_path = argv[1];  // Use provided database path
	if (argc >= 3)
		sock_path = argv[2];  // Use provided socket path
	if (argc > 3) {
		usage(argv[0]);
		return 1;
	}

	// Load domain database from file
	load_db(db_path);
	printf("[dns_server] Loaded %zu records from %s\n", g_entry_count, db_path);

	// Clean up any leftover socket file from previous run
	cleanup_socket();
	
	// Register signal handler for Ctrl+C
	signal(SIGINT, on_sigint);

	// Create a UNIX domain socket (AF_UNIX) for IPC communication
	g_listen_fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (g_listen_fd < 0)
		dns_die("socket");

	// Set up socket address structure
	struct sockaddr_un addr;
	memset(&addr, 0, sizeof(addr));           // Clear the structure
	addr.sun_family = AF_UNIX;                 // Use UNIX domain sockets
	strncpy(addr.sun_path, sock_path, sizeof(addr.sun_path) - 1);  // Set socket file path

	// Bind socket to the specified path
	if (bind(g_listen_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
		dns_die("bind");
	
	// Mark socket as a listening socket with a backlog of 8 pending connections
	if (listen(g_listen_fd, 8) < 0)
		dns_die("listen");

	printf("[dns_server] Listening on UNIX socket %s\n", sock_path);

	// Main server loop: accept and handle clients forever
	for (;;) {
		// Wait for a client connection
		int client_fd = accept(g_listen_fd, NULL, NULL);
		if (client_fd < 0) {
			// If interrupted by signal, try again
			if (errno == EINTR)
				continue;
			dns_die("accept");  // Other error, exit
		}
		
		// Handle this client's requests
		handle_client(client_fd);
		
		// Close the client socket
		close(client_fd);
	}
}
