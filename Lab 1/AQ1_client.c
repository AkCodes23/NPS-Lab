#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define SERVER_IP "127.0.0.1"
#define TCP_PORT 11000
#define UDP_PORT 11001
#define BUFFER_SIZE 256

// Function to handle TCP client
void* tcp_client(void *arg) {
    int sock;
    struct sockaddr_in server_addr, local_addr;
    socklen_t local_addr_len;
    char message[BUFFER_SIZE];
    char buffer[BUFFER_SIZE];

    // Create TCP socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("TCP socket");
        pthread_exit(NULL);
    }

    // Connect to server
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(TCP_PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    printf("\n=== TCP Client (Connection-Oriented) ===\n");
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("TCP connect");
        pthread_exit(NULL);
    }

    printf("Connected to server %s:%d\n", SERVER_IP, TCP_PORT);

    // Get local socket information
    local_addr_len = sizeof(local_addr);
    getsockname(sock, (struct sockaddr *)&local_addr, &local_addr_len);

    printf("CLIENT SOCKET INFORMATION:\n");
    printf("  IP Address: %s\n", inet_ntoa(local_addr.sin_addr));
    printf("  Port Number: %d\n", ntohs(local_addr.sin_port));

    // Send message
    strcpy(message, "Hello from TCP Client");
    if (send(sock, message, strlen(message), 0) < 0) {
        perror("TCP send");
    } else {
        printf("Sent message: %s\n", message);
    }

    // Receive response
    int recv_len = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    if (recv_len > 0) {
        buffer[recv_len] = '\0';
        printf("Received from server: %s\n", buffer);
    }

    close(sock);
    printf("TCP connection closed\n");
    pthread_exit(NULL);
}

// Function to handle UDP client
void* udp_client(void *arg) {
    int sock;
    struct sockaddr_in server_addr, local_addr;
    socklen_t server_addr_len, local_addr_len;
    char message[BUFFER_SIZE];
    char buffer[BUFFER_SIZE];

    // Create UDP socket
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("UDP socket");
        pthread_exit(NULL);
    }

    // Set server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(UDP_PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    printf("\n=== UDP Client (Connectionless) ===\n");
    printf("Communicating with server %s:%d\n", SERVER_IP, UDP_PORT);

    // Get local socket information (before send)
    local_addr_len = sizeof(local_addr);
    getsockname(sock, (struct sockaddr *)&local_addr, &local_addr_len);

    printf("CLIENT SOCKET INFORMATION (before send):\n");
    printf("  IP Address: %s\n", inet_ntoa(local_addr.sin_addr));
    printf("  Port Number: %d\n", ntohs(local_addr.sin_port));

    // Send message
    strcpy(message, "Hello from UDP Client");
    server_addr_len = sizeof(server_addr);
    if (sendto(sock, message, strlen(message), 0,
               (struct sockaddr *)&server_addr, server_addr_len) < 0) {
        perror("UDP sendto");
    } else {
        printf("Sent message: %s\n", message);
    }

    // Get socket info after send
    local_addr_len = sizeof(local_addr);
    getsockname(sock, (struct sockaddr *)&local_addr, &local_addr_len);

    printf("CLIENT SOCKET INFORMATION (after send):\n");
    printf("  IP Address: %s\n", inet_ntoa(local_addr.sin_addr));
    printf("  Port Number: %d\n", ntohs(local_addr.sin_port));

    // Receive response
    int recv_len = recvfrom(sock, buffer, BUFFER_SIZE - 1, 0,
                            (struct sockaddr *)&server_addr, &server_addr_len);
    if (recv_len > 0) {
        buffer[recv_len] = '\0';
        printf("Received from server: %s\n", buffer);
    }

    close(sock);
    printf("UDP communication closed\n");
    pthread_exit(NULL);
}

int main() {
    pthread_t tcp_thread, udp_thread;

    printf("=== UNIX Socket Client (TCP and UDP) ===\n");
    printf("Starting TCP and UDP clients...\n");

    sleep(2);  // Give server time to start

    // Create TCP client thread
    if (pthread_create(&tcp_thread, NULL, tcp_client, NULL) != 0) {
        perror("pthread_create TCP");
        exit(1);
    }

    sleep(1);

    // Create UDP client thread
    if (pthread_create(&udp_thread, NULL, udp_client, NULL) != 0) {
        perror("pthread_create UDP");
        exit(1);
    }

    // Wait for threads
    pthread_join(tcp_thread, NULL);
    pthread_join(udp_thread, NULL);

    printf("\nAll clients finished\n");
    return 0;
}
