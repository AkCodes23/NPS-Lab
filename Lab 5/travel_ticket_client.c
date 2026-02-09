#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 14000
#define SERVER_IP "127.0.0.1"
#define BUFFER_SIZE 256

// Request structure
typedef struct {
    int route_choice;  // 1 or 2 for the two routes
    int seats_requested;
} ReservationRequest;

// Response structure
typedef struct {
    int status;  // 1: Success, 0: Failure
    char message[200];
    int available_seats;
    int booked_seats;
} ReservationResponse;

void display_menu() {
    printf("\n========== AVAILABLE ROUTES ==========\n");
    printf("Route 1: Delhi -> Mumbai\n");
    printf("Route 2: Bangalore -> Hyderabad\n");
    printf("=====================================\n");
}

int main() {
    int sock;
    struct sockaddr_in server_addr;
    ReservationRequest req;
    ReservationResponse resp;
    char welcome[BUFFER_SIZE];
    int bytes_recv;

    printf("=== Travel Ticket Reservation Client ===\n\n");

    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        exit(1);
    }

    // Connect to server
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    printf("Connecting to server %s:%d...\n", SERVER_IP, PORT);
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        exit(1);
    }

    printf("Connected successfully!\n\n");

    // Receive welcome message from server
    bytes_recv = recv(sock, welcome, BUFFER_SIZE - 1, 0);
    if (bytes_recv > 0) {
        welcome[bytes_recv] = '\0';
        printf("%s\n", welcome);
    }

    // Get route choice from user
    display_menu();
    printf("Enter route choice (1 or 2): ");
    scanf("%d", &req.route_choice);

    // Get number of seats
    printf("Enter number of seats to book: ");
    scanf("%d", &req.seats_requested);

    printf("\nProcessing reservation request...\n");

    // Send request to server
    if (send(sock, &req, sizeof(ReservationRequest), 0) < 0) {
        perror("send");
        close(sock);
        exit(1);
    }

    // Receive response from server
    if (recv(sock, &resp, sizeof(ReservationResponse), 0) <= 0) {
        printf("Error: Connection closed by server\n");
        close(sock);
        exit(1);
    }

    // Display response
    printf("\n========== SERVER RESPONSE ==========\n");
    printf("%s\n", resp.message);

    if (resp.status == 1) {
        printf("\nReservation Status: ✓ SUCCESSFUL\n");
        printf("Available Seats for Route %d: %d\n", req.route_choice, resp.available_seats);
        printf("Booked Seats for Route %d: %d\n", req.route_choice, resp.booked_seats);
    } else {
        printf("\nReservation Status: ✗ FAILED\n");
        printf("Available Seats for Route %d: %d\n", req.route_choice, resp.available_seats);
    }
    printf("=====================================\n");

    printf("\nThank you for using Travel Ticket Reservation System!\n");

    close(sock);
    return 0;
}
