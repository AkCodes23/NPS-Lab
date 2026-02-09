#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 14000
#define BUFFER_SIZE 256

// Route structure
typedef struct {
    int route_id;
    char source[50];
    char destination[50];
    int total_seats;
    int available_seats;
    int booked_seats;
} Route;

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

// Global routes database
Route routes[2];
pthread_mutex_t db_lock = PTHREAD_MUTEX_INITIALIZER;

// Function to initialize routes
void initialize_routes() {
    // Route 1: Delhi to Mumbai
    routes[0].route_id = 1;
    strcpy(routes[0].source, "Delhi");
    strcpy(routes[0].destination, "Mumbai");
    routes[0].total_seats = 50;
    routes[0].available_seats = 50;
    routes[0].booked_seats = 0;

    // Route 2: Bangalore to Hyderabad
    routes[1].route_id = 2;
    strcpy(routes[1].source, "Bangalore");
    strcpy(routes[1].destination, "Hyderabad");
    routes[1].total_seats = 40;
    routes[1].available_seats = 40;
    routes[1].booked_seats = 0;
}

// Function to display route information
void display_route_info(Route *route) {
    printf("  Route %d: %s -> %s\n", route->route_id, route->source, route->destination);
    printf("    Total Seats: %d | Available: %d | Booked: %d\n",
           route->total_seats, route->available_seats, route->booked_seats);
}

// Function to handle seat reservation
int reserve_seats(int route_choice, int seats_requested, ReservationResponse *resp) {
    if (route_choice < 1 || route_choice > 2) {
        resp->status = 0;
        sprintf(resp->message, "Invalid route choice!");
        return 0;
    }

    pthread_mutex_lock(&db_lock);

    Route *route = &routes[route_choice - 1];

    printf("Client requesting %d seats for Route %d (%s -> %s)\n",
           seats_requested, route->route_id, route->source, route->destination);

    if (seats_requested <= 0) {
        resp->status = 0;
        sprintf(resp->message, "Invalid number of seats requested!");
        resp->available_seats = route->available_seats;
        resp->booked_seats = route->booked_seats;
        pthread_mutex_unlock(&db_lock);
        return 0;
    }

    if (seats_requested > route->available_seats) {
        resp->status = 0;
        sprintf(resp->message, "Sorry! Only %d seats available. Cannot book %d seats.",
                route->available_seats, seats_requested);
        resp->available_seats = route->available_seats;
        resp->booked_seats = route->booked_seats;
        printf("  FAILED: Insufficient seats (Available: %d, Requested: %d)\n",
               route->available_seats, seats_requested);
        pthread_mutex_unlock(&db_lock);
        return 0;
    }

    // Book the seats
    route->available_seats -= seats_requested;
    route->booked_seats += seats_requested;

    resp->status = 1;
    sprintf(resp->message, "SUCCESS! %d seat(s) booked for %s -> %s (Confirmation #%d)",
            seats_requested, route->source, route->destination,
            (rand() % 10000) + 1000);
    resp->available_seats = route->available_seats;
    resp->booked_seats = route->booked_seats;

    printf("  SUCCESS: %d seats booked (Remaining: %d)\n",
           seats_requested, route->available_seats);

    pthread_mutex_unlock(&db_lock);
    return 1;
}

// Function to display current status
void display_all_routes() {
    printf("\n=== TICKET RESERVATION SERVER STATUS ===\n");
    for (int i = 0; i < 2; i++) {
        display_route_info(&routes[i]);
    }
    printf("=========================================\n\n");
}

// Thread function to handle each client
void* handle_client(void *arg) {
    int client_fd = *(int *)arg;
    free(arg);

    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    getpeername(client_fd, (struct sockaddr *)&client_addr, &client_addr_len);

    printf("\n--- Client Connected: %s:%d ---\n", 
           inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    // Send welcome message with route information
    char welcome[BUFFER_SIZE];
    sprintf(welcome, "WELCOME TO TICKET RESERVATION SYSTEM\nRoute 1: Delhi -> Mumbai (Seats: 50)\nRoute 2: Bangalore -> Hyderabad (Seats: 40)\n");
    send(client_fd, welcome, strlen(welcome), 0);

    // Receive reservation request
    ReservationRequest req;
    if (recv(client_fd, &req, sizeof(ReservationRequest), 0) <= 0) {
        printf("Client disconnected before sending request\n");
        close(client_fd);
        pthread_exit(NULL);
    }

    printf("Received request: Route %d, Seats %d\n", req.route_choice, req.seats_requested);

    // Process reservation
    ReservationResponse resp;
    reserve_seats(req.route_choice, req.seats_requested, &resp);

    // Send response
    if (send(client_fd, &resp, sizeof(ReservationResponse), 0) < 0) {
        perror("send");
    }

    printf("Response sent to client: %s\n", resp.message);
    printf("--- Client Disconnected ---\n");

    close(client_fd);
    pthread_exit(NULL);
}

int main() {
    int server_fd, *client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len;
    pthread_t thread_id;

    // Initialize routes
    initialize_routes();
    display_all_routes();

    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        exit(1);
    }

    // Set socket option to reuse address
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Bind socket
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        exit(1);
    }

    // Listen for incoming connections
    if (listen(server_fd, 10) < 0) {
        perror("listen");
        exit(1);
    }

    printf("=== Travel Ticket Reservation Server ===\n");
    printf("Server listening on port %d...\n", PORT);
    printf("Waiting for client connections...\n\n");

    while (1) {
        // Accept connection
        client_addr_len = sizeof(client_addr);
        int new_client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);

        if (new_client_fd < 0) {
            perror("accept");
            continue;
        }

        // Create thread to handle client
        client_fd = malloc(sizeof(int));
        *client_fd = new_client_fd;

        if (pthread_create(&thread_id, NULL, handle_client, client_fd) != 0) {
            perror("pthread_create");
            close(new_client_fd);
            free(client_fd);
        } else {
            pthread_detach(thread_id);
        }

        // Display updated status
        display_all_routes();
    }

    close(server_fd);
    return 0;
}
