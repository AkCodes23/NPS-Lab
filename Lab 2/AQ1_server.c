#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define TCP_PORT 10000
#define UDP_PORT 10001
#define BUFFER_SIZE 256

// Function to handle TCP connections
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

    printf("\n=== TCP Server ===\n");
    printf("Listening on port %d...\n", TCP_PORT);

    while (1) {
        client_addr_len = sizeof(client_addr);
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);

        if (client_fd < 0) {
            perror("TCP accept");
            continue;
        }

        printf("\n--- TCP Connection Established ---\n");
        printf("Client Socket Address (struct):\n");
        printf("  Socket Family: AF_INET (IPv4)\n");
        printf("  IP Address: %s\n", inet_ntoa(client_addr.sin_addr));
        printf("  Port Number: %d\n", ntohs(client_addr.sin_port));

        // Get local socket information
        struct sockaddr_in local_addr;
        socklen_t local_addr_len = sizeof(local_addr);
        getsockname(client_fd, (struct sockaddr *)&local_addr, &local_addr_len);

        printf("Server Socket Address (struct):\n");
        printf("  IP Address: %s\n", inet_ntoa(local_addr.sin_addr));
        printf("  Port Number: %d\n", ntohs(local_addr.sin_port));

        // Receive data
        int recv_len = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
        if (recv_len > 0) {
            buffer[recv_len] = '\0';
            printf("Received from client: %s\n", buffer);

            // Send response
            char response[BUFFER_SIZE];
            sprintf(response, "Server received: %s", buffer);
            send(client_fd, response, strlen(response), 0);
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

    printf("\n=== UDP Server ===\n");
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
        printf("Client Socket Address (struct):\n");
        printf("  Socket Family: AF_INET (IPv4)\n");
        printf("  IP Address: %s\n", inet_ntoa(client_addr.sin_addr));
        printf("  Port Number: %d\n", ntohs(client_addr.sin_port));

        // Get local socket information
        struct sockaddr_in local_addr;
        socklen_t local_addr_len = sizeof(local_addr);
        getsockname(server_fd, (struct sockaddr *)&local_addr, &local_addr_len);

        printf("Server Socket Address (struct):\n");
        printf("  IP Address: %s\n", inet_ntoa(local_addr.sin_addr));
        printf("  Port Number: %d\n", ntohs(local_addr.sin_port));

        printf("Received from client: %s\n", buffer);

        // Send response
        char response[BUFFER_SIZE];
        sprintf(response, "Server received: %s", buffer);
        sendto(server_fd, response, strlen(response), 0,
               (struct sockaddr *)&client_addr, client_addr_len);

        if (strcmp(buffer, "quit") == 0) {
            printf("Client quit, ready for next connection\n");
        }
    }

    close(server_fd);
    pthread_exit(NULL);
}

int main() {
    pthread_t tcp_thread, udp_thread;

    printf("=== Combined TCP/UDP Server ===\n");
    printf("Starting both TCP and UDP servers...\n");

    // Create TCP handler thread
    if (pthread_create(&tcp_thread, NULL, tcp_handler, NULL) != 0) {
        perror("pthread_create TCP");
        exit(1);
    }

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
