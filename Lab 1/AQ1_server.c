/*
 * ============================================================================
 * LAB-1 ADDITIONAL EXERCISE Q1: UNIX Socket Server (TCP and UDP)
 * ============================================================================
 * 
 * PROGRAM DESCRIPTION:
 * This program implements a multi-threaded server that simultaneously handles
 * both TCP (connection-oriented) and UDP (connectionless) socket connections.
 * The server displays client socket information including:
 *   - Client IP address
 *   - Client port number
 *   - Server IP address
 *   - Server port number
 * 
 * TCP SERVER (Port 11000):
 *   - Accepts client connections
 *   - Receives data from clients
 *   - Displays complete client and server socket information
 *   - Sends response back to client
 * 
 * UDP SERVER (Port 11001):
 *   - Receives datagrams from clients
 *   - Displays client and server socket information for each message
 *   - Sends response back to client
 * 
 * The server runs both services concurrently using POSIX threads (pthreads).
 * ============================================================================
 */

#include <stdio.h>              // Standard I/O library
#include <stdlib.h>             // Standard library (malloc, exit, etc.)
#include <string.h>             // String manipulation functions
#include <unistd.h>             // Unix standard functions (close, etc.)
#include <sys/types.h>          // Data types for socket API
#include <sys/socket.h>         // Socket API functions
#include <netinet/in.h>         // Internet address family structures
#include <arpa/inet.h>          // IP address conversion functions (inet_ntoa)
#include <pthread.h>            // POSIX thread library for multi-threading

#define TCP_PORT 11000          // Port number for TCP server
#define UDP_PORT 11001          // Port number for UDP server
#define BUFFER_SIZE 256         // Size of data buffer for sending/receiving

/*
 * FUNCTION: tcp_handler
 * PURPOSE: Thread function to handle TCP socket operations
 * This function runs in a separate thread and:
 *   1. Creates a TCP socket
 *   2. Binds it to TCP_PORT
 *   3. Listens for incoming connections
 *   4. Accepts connections and displays socket information
 *   5. Receives data from clients and sends responses
 */
void* tcp_handler(void *arg) {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len;
    char buffer[BUFFER_SIZE];

    // Create TCP socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("TCP socket");
        pthread_exit(NULL);
    }

    // Set socket option to reuse address
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Bind
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(TCP_PORT);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("TCP bind");
        close(server_fd);
        pthread_exit(NULL);
    }

    // Listen
    if (listen(server_fd, 5) < 0) {
        perror("TCP listen");
        close(server_fd);
        pthread_exit(NULL);
    }

    printf("\n=== TCP Server (Connection-Oriented) ===\n");
    printf("Listening on port %d...\n", TCP_PORT);

    while (1) {
        client_addr_len = sizeof(client_addr);
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);

        if (client_fd < 0) {
            perror("TCP accept");
            continue;
        }

        printf("\n--- TCP Connection Established ---\n");
        printf("CLIENT SOCKET INFORMATION:\n");
        printf("  Socket Address Family: AF_INET (IPv4)\n");
        printf("  IP Address: %s\n", inet_ntoa(client_addr.sin_addr));
        printf("  Port Number: %d\n", ntohs(client_addr.sin_port));

        // Get local socket information
        struct sockaddr_in local_addr;
        socklen_t local_addr_len = sizeof(local_addr);
        getsockname(client_fd, (struct sockaddr *)&local_addr, &local_addr_len);

        printf("SERVER SOCKET INFORMATION:\n");
        printf("  IP Address: %s\n", inet_ntoa(local_addr.sin_addr));
        printf("  Port Number: %d\n", ntohs(local_addr.sin_port));

        // Receive data
        int recv_len = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
        if (recv_len > 0) {
            buffer[recv_len] = '\0';
            printf("Data received from client: %s\n", buffer);

            // Send response
            char response[BUFFER_SIZE];
            sprintf(response, "TCP Server received: %s", buffer);
            send(client_fd, response, strlen(response), 0);
            printf("Response sent to client\n");
        }

        close(client_fd);
        printf("TCP Connection closed\n");
    }

    close(server_fd);
    pthread_exit(NULL);
}

// Function to handle UDP messages
void* udp_handler(void *arg) {
    int server_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len;
    char buffer[BUFFER_SIZE];

    // Create UDP socket
    server_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_fd < 0) {
        perror("UDP socket");
        pthread_exit(NULL);
    }

    // Bind
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(UDP_PORT);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("UDP bind");
        close(server_fd);
        pthread_exit(NULL);
    }

    printf("\n=== UDP Server (Connectionless) ===\n");
    printf("Listening on port %d...\n", UDP_PORT);

    while (1) {
        client_addr_len = sizeof(client_addr);
        int recv_len = recvfrom(server_fd, buffer, BUFFER_SIZE - 1, 0,
                                (struct sockaddr *)&client_addr, &client_addr_len);

        if (recv_len < 0) {
            perror("UDP recvfrom");
            continue;
        }

        buffer[recv_len] = '\0';

        printf("\n--- UDP Message Received ---\n");
        printf("CLIENT SOCKET INFORMATION:\n");
        printf("  Socket Address Family: AF_INET (IPv4)\n");
        printf("  IP Address: %s\n", inet_ntoa(client_addr.sin_addr));
        printf("  Port Number: %d\n", ntohs(client_addr.sin_port));

        // Get local socket information
        struct sockaddr_in local_addr;
        socklen_t local_addr_len = sizeof(local_addr);
        getsockname(server_fd, (struct sockaddr *)&local_addr, &local_addr_len);

        printf("SERVER SOCKET INFORMATION:\n");
        printf("  IP Address: %s\n", inet_ntoa(local_addr.sin_addr));
        printf("  Port Number: %d\n", ntohs(local_addr.sin_port));

        printf("Data received from client: %s\n", buffer);

        // Send response
        char response[BUFFER_SIZE];
        sprintf(response, "UDP Server received: %s", buffer);
        sendto(server_fd, response, strlen(response), 0,
               (struct sockaddr *)&client_addr, client_addr_len);
        printf("Response sent to client\n");
    }

    close(server_fd);
    pthread_exit(NULL);
}

int main() {
    pthread_t tcp_thread, udp_thread;

    printf("=== UNIX Socket Server (TCP and UDP) ===\n");
    printf("Starting TCP and UDP servers...\n");

    // Create TCP handler thread
    if (pthread_create(&tcp_thread, NULL, tcp_handler, NULL) != 0) {
        perror("pthread_create TCP");
        exit(1);
    }

    sleep(1);

    // Create UDP handler thread
    if (pthread_create(&udp_thread, NULL, udp_handler, NULL) != 0) {
        perror("pthread_create UDP");
        exit(1);
    }

    // Wait for threads
    pthread_join(tcp_thread, NULL);
    pthread_join(udp_thread, NULL);

    return 0;
}
