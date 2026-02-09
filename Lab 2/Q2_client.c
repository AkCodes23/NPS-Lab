#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 9999
#define SERVER_IP "127.0.0.1"
#define BUFFER_SIZE 1024

typedef struct {
    char string[BUFFER_SIZE];
    int is_palindrome;
    int length;
    int vowel_count[5];  // a, e, i, o, u
    int total_vowels;
} Response;

int main() {
    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    Response resp;
    socklen_t addr_len;

    // Create UDP socket
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket");
        exit(1);
    }

    // Set server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    printf("UDP Client started\n");
    printf("Connected to server %s:%d\n", SERVER_IP, PORT);
    printf("Enter strings to check (enter 'Halt' to exit):\n\n");

    while (1) {
        printf("Enter a string: ");
        fgets(buffer, BUFFER_SIZE - 1, stdin);

        // Remove newline if present
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }

        // Send string to server
        if (sendto(sock, buffer, strlen(buffer), 0,
                   (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            perror("sendto");
            break;
        }

        // Receive response from server
        addr_len = sizeof(server_addr);
        int recv_len = recvfrom(sock, &resp, sizeof(Response), 0,
                                (struct sockaddr *)&server_addr, &addr_len);
        if (recv_len < 0) {
            perror("recvfrom");
            break;
        }

        // Check for exit condition
        if (strcmp(buffer, "Halt") == 0) {
            printf("Halting client...\n");
            break;
        }

        // Display results
        printf("\n--- Server Response ---\n");
        printf("String: %s\n", resp.string);
        printf("Length: %d\n", resp.length);
        printf("Palindrome: %s\n", resp.is_palindrome ? "Yes" : "No");
        printf("Vowel Occurrences:\n");
        printf("  A: %d\n", resp.vowel_count[0]);
        printf("  E: %d\n", resp.vowel_count[1]);
        printf("  I: %d\n", resp.vowel_count[2]);
        printf("  O: %d\n", resp.vowel_count[3]);
        printf("  U: %d\n", resp.vowel_count[4]);
        printf("  Total: %d\n", resp.total_vowels);
        printf("\n");
    }

    close(sock);
    return 0;
}
