#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8888
#define SERVER_IP "127.0.0.1"
#define MAX_INTEGERS 100
#define BUFFER_SIZE 1024

typedef struct {
    int choice;
    int count;
    int numbers[MAX_INTEGERS];
} Request;

typedef struct {
    int result_size;
    int results[MAX_INTEGERS];
} Response;

void display_results(Response *resp, int choice) {
    printf("\n--- Results ---\n");

    if (choice == 1) {  // Search
        if (resp->results[0] == 1) {
            printf("Number found at index: %d\n", resp->results[1]);
        } else {
            printf("Number not found\n");
        }

    } else if (choice == 2 || choice == 3) {  // Sort
        printf("Sorted numbers: ");
        for (int i = 0; i < resp->result_size; i++) {
            printf("%d ", resp->results[i]);
        }
        printf("\n");

    } else if (choice == 4) {  // Split Odd/Even
        printf("Numbers (Even first, then Odd): ");
        for (int i = 0; i < resp->result_size; i++) {
            printf("%d ", resp->results[i]);
        }
        printf("\n");
    }
}

int main() {
    int sock;
    struct sockaddr_in server_addr;
    Request req;
    Response resp;

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

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        exit(1);
    }

    printf("Connected to server %s:%d\n\n", SERVER_IP, PORT);

    while (1) {
        printf("\n--- Menu ---\n");
        printf("1. Search for a number\n");
        printf("2. Sort in ascending order\n");
        printf("3. Sort in descending order\n");
        printf("4. Split into odd and even\n");
        printf("5. Exit\n");
        printf("Enter your choice: ");

        int choice;
        scanf("%d", &choice);

        if (choice == 5) {  // Exit
            req.choice = 5;
            req.count = 0;
            send(sock, &req, sizeof(Request), 0);
            printf("Exiting...\n");
            break;
        }

        if (choice < 1 || choice > 5) {
            printf("Invalid choice! Please try again.\n");
            continue;
        }

        printf("Enter the number of integers (max %d): ", MAX_INTEGERS);
        scanf("%d", &req.count);

        if (req.count <= 0 || req.count > MAX_INTEGERS) {
            printf("Invalid count!\n");
            continue;
        }

        printf("Enter %d integers: ", req.count);
        for (int i = 0; i < req.count; i++) {
            scanf("%d", &req.numbers[i]);
        }

        // For search, take the last number as the search target
        if (choice == 1) {
            printf("(The last number entered will be searched)\n");
        }

        req.choice = choice;

        // Send request
        if (send(sock, &req, sizeof(Request), 0) < 0) {
            perror("send");
            break;
        }

        // Receive response
        if (recv(sock, &resp, sizeof(Response), 0) <= 0) {
            printf("Connection closed by server\n");
            break;
        }

        display_results(&resp, choice);
    }

    close(sock);
    return 0;
}
