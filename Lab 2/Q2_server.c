#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>

#define PORT 9999
#define BUFFER_SIZE 1024

typedef struct {
    char string[BUFFER_SIZE];
    int is_palindrome;
    int length;
    int vowel_count[5];  // a, e, i, o, u
    int total_vowels;
} Response;

int is_palindrome(char *str) {
    int left = 0, right = strlen(str) - 1;
    
    while (left < right) {
        // Skip non-alphabetic characters
        while (left < right && !isalpha(str[left])) left++;
        while (left < right && !isalpha(str[right])) right--;
        
        // Compare ignoring case
        if (tolower(str[left]) != tolower(str[right])) {
            return 0;
        }
        left++;
        right--;
    }
    return 1;
}

void count_vowels(char *str, int *vowel_count, int *total) {
    // Initialize vowel count: a, e, i, o, u
    vowel_count[0] = 0;  // a
    vowel_count[1] = 0;  // e
    vowel_count[2] = 0;  // i
    vowel_count[3] = 0;  // o
    vowel_count[4] = 0;  // u
    *total = 0;

    for (int i = 0; str[i] != '\0'; i++) {
        char c = tolower(str[i]);
        if (c == 'a') {
            vowel_count[0]++;
            (*total)++;
        } else if (c == 'e') {
            vowel_count[1]++;
            (*total)++;
        } else if (c == 'i') {
            vowel_count[2]++;
            (*total)++;
        } else if (c == 'o') {
            vowel_count[3]++;
            (*total)++;
        } else if (c == 'u') {
            vowel_count[4]++;
            (*total)++;
        }
    }
}

int main() {
    int server_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len;
    char buffer[BUFFER_SIZE];
    Response resp;

    // Create UDP socket
    server_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_fd < 0) {
        perror("socket");
        exit(1);
    }

    // Bind socket
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        exit(1);
    }

    printf("UDP Server listening on port %d...\n", PORT);
    printf("Waiting for strings. (Client should send 'Halt' to terminate)\n\n");

    while (1) {
        client_addr_len = sizeof(client_addr);

        // Receive data from client
        int recv_len = recvfrom(server_fd, buffer, BUFFER_SIZE - 1, 0,
                                (struct sockaddr *)&client_addr, &client_addr_len);
        if (recv_len < 0) {
            perror("recvfrom");
            continue;
        }

        buffer[recv_len] = '\0';
        printf("Received from %s:%d: \"%s\"\n", 
               inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), buffer);

        // Check for exit condition
        if (strcmp(buffer, "Halt") == 0) {
            printf("Client sent 'Halt', terminating connection with this client\n\n");
            // Send confirmation
            strcpy(resp.string, buffer);
            resp.length = strlen(buffer);
            resp.is_palindrome = 0;
            resp.total_vowels = 0;
            sendto(server_fd, &resp, sizeof(Response), 0,
                   (struct sockaddr *)&client_addr, client_addr_len);
            continue;
        }

        // Check if palindrome
        resp.is_palindrome = is_palindrome(buffer);

        // Count vowels
        count_vowels(buffer, resp.vowel_count, &resp.total_vowels);

        // Store string and length
        strcpy(resp.string, buffer);
        resp.length = strlen(buffer);

        printf("Palindrome: %s, Length: %d\n", 
               resp.is_palindrome ? "Yes" : "No", resp.length);
        printf("Vowels - A:%d, E:%d, I:%d, O:%d, U:%d, Total:%d\n",
               resp.vowel_count[0], resp.vowel_count[1], resp.vowel_count[2],
               resp.vowel_count[3], resp.vowel_count[4], resp.total_vowels);

        // Send response back to client
        if (sendto(server_fd, &resp, sizeof(Response), 0,
                   (struct sockaddr *)&client_addr, client_addr_len) < 0) {
            perror("sendto");
        }
        printf("Response sent back to client\n\n");
    }

    close(server_fd);
    return 0;
}
