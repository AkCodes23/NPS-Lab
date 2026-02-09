/*
 * ============================================================================
 * LAB-2 QUESTION 1: TCP-Based Integer Operations Server
 * ============================================================================
 * 
 * PROGRAM DESCRIPTION:
 * This is a TCP socket server that performs mathematical operations on sets
 * of integers received from clients. The server supports the following operations:
 *   1. SEARCH: Find a number in an array (returns index if found)
 *   2. SORT ASCENDING: Sort array in ascending order
 *   3. SORT DESCENDING: Sort array in descending order
 *   4. SPLIT ODD/EVEN: Separate odd and even numbers (evens first, then odds)
 *   5. EXIT: Close client connection
 * 
 * PROTOCOL:
 * - Uses TCP (SOCK_STREAM) on port 8888 for reliable connection-oriented communication
 * - Receives Request structure containing operation choice and integer array
 * - Processes request based on choice
 * - Sends back Response structure with results
 * - Client can perform multiple operations until choosing EXIT
 * 
 * KEY FEATURES:
 * - Handles multiple clients sequentially (one at a time)
 * - Uses qsort() for efficient sorting
 * - All results returned in a single Response structure
 * ============================================================================
 */

#include <stdio.h>              // Standard I/O functions
#include <stdlib.h>             // Standard library (malloc, qsort, exit, etc.)
#include <string.h>             // String functions (memcpy, sprintf, etc.)
#include <unistd.h>             // Unix I/O functions (close, etc.)
#include <sys/types.h>          // Data types for socket API
#include <sys/socket.h>         // Socket API functions (socket, bind, listen, etc.)
#include <netinet/in.h>         // IPv4 socket structures (sockaddr_in)
#include <arpa/inet.h>          // IP address conversion functions (inet_ntoa, htons)

#define PORT 8888               // TCP port number for server
#define MAX_INTEGERS 100        // Maximum number of integers in array
#define BUFFER_SIZE 1024        // Size of communication buffer

/*
 * Data structure to receive client requests
 * This struct is sent from client to server containing:
 *   - choice: Operation to perform (1-5)
 *   - count: Number of integers in the array
 *   - numbers: Array of integers to process
 */
typedef struct {
    int choice;                 // Operation choice (1=search, 2=asc sort, 3=desc sort, 4=split)
    int count;                  // Number of integers provided
    int numbers[MAX_INTEGERS];  // Array of integers
} Request;

/*
 * Data structure to send server response to client
 * Contains:
 *   - result_size: Number of results returned
 *   - results: Array of result integers
 */
typedef struct {
    int result_size;            // Number of results in the results array
    int results[MAX_INTEGERS];  // Results of the operation
} Response;

/*
 * FUNCTION: search_number
 * PURPOSE: Search for a number in array using linear search
 * PARAMETERS:
 *   - arr: Pointer to integer array to search
 *   - size: Number of elements in array
 *   - num: Number to search for
 * RETURNS: Index of element if found, -1 if not found
 */
int search_number(int *arr, int size, int num) {
    for (int i = 0; i < size; i++) {
        // Check if current element matches the search number
        if (arr[i] == num) {
            return i;  // Return index when found
        }
    }
    return -1;  // Return -1 if not found in entire array
}

/*
 * FUNCTION: compare_asc
 * PURPOSE: Comparison function for qsort - sorts in ascending order
 * Used by qsort() to determine element ordering
 * Returns: negative if a < b, 0 if a == b, positive if a > b
 */
int compare_asc(const void *a, const void *b) {
    return (*(int *)a - *(int *)b);  // Subtract: puts smaller values first
}

/*
 * FUNCTION: compare_desc
 * PURPOSE: Comparison function for qsort - sorts in descending order
 * Returns: negative if a > b, 0 if a == b, positive if a < b
 */
int compare_desc(const void *a, const void *b) {
    return (*(int *)b - *(int *)a);  // Reverse subtraction: puts larger values first
}

/*
 * FUNCTION: process_request
 * PURPOSE: Main processing function - handles all operations
 * PARAMETERS:
 *   - req: Pointer to Request structure containing operation and data
 * RETURNS: Pointer to Response structure with results
 * 
 * This function:
 * 1. Allocates memory for response
 * 2. Based on operation choice, performs appropriate operation
 * 3. Stores results in response structure
 * 4. Returns response to be sent to client
 */
Response* process_request(Request *req) {
    // Allocate memory for response structure
    Response *resp = malloc(sizeof(Response));
    resp->result_size = 0;  // Initialize result size to 0

    if (req->choice == 1) {  
        // OPERATION 1: SEARCH - Find a number in the array
        printf("Server: Searching for number in %d integers\n", req->count);
        
        // Search for the last number entered (used as search key)
        int index = search_number(req->numbers, req->count, req->numbers[MAX_INTEGERS - 1]);
        if (index != -1) {
            resp->results[0] = 1;  // Flag: Found = 1
            resp->results[1] = index;  // Store the index where found
        } else {
            resp->results[0] = 0;  // Flag: Not found = 0
        }
        resp->result_size = 2;  // Return 2 values (found flag + index/unused)

    } else if (req->choice == 2) {  
        // OPERATION 2: SORT ASCENDING
        printf("Server: Sorting %d integers in ascending order\n", req->count);
        
        // Copy input numbers to result array
        memcpy(resp->results, req->numbers, req->count * sizeof(int));
        
        // Use qsort (C standard library) to sort in ascending order
        // Parameters: array, number of elements, size of each element, comparison function
        qsort(resp->results, req->count, sizeof(int), compare_asc);
        resp->result_size = req->count;  // Number of sorted results

    } else if (req->choice == 3) {  // Sort Descending
        printf("Server: Sorting %d integers in descending order\n", req->count);
        memcpy(resp->results, req->numbers, req->count * sizeof(int));
        qsort(resp->results, req->count, sizeof(int), compare_desc);
        resp->result_size = req->count;

    } else if (req->choice == 4) {  // Split Odd/Even
        printf("Server: Splitting %d integers into odd and even\n", req->count);
        int idx = 0;
        // Add even numbers first
        for (int i = 0; i < req->count; i++) {
            if (req->numbers[i] % 2 == 0) {
                resp->results[idx++] = req->numbers[i];
            }
        }
        // Add odd numbers
        for (int i = 0; i < req->count; i++) {
            if (req->numbers[i] % 2 != 0) {
                resp->results[idx++] = req->numbers[i];
            }
        }
        resp->result_size = req->count;
    }

    return resp;
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len;
    Request req;
    Response *resp;

    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        exit(1);
    }

    // Set socket options to reuse address
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
    if (listen(server_fd, 5) < 0) {
        perror("listen");
        exit(1);
    }

    printf("Server listening on port %d...\n", PORT);

    while (1) {
        // Accept connection
        client_addr_len = sizeof(client_addr);
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_fd < 0) {
            perror("accept");
            continue;
        }

        printf("Client connected: %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        // Process requests from client
        while (1) {
            // Receive request
            if (recv(client_fd, &req, sizeof(Request), 0) <= 0) {
                break;  // Client disconnected
            }

            printf("Received request with choice: %d, count: %d\n", req.choice, req.count);

            // Check for exit condition
            if (req.choice == 5) {  // Exit choice
                printf("Client requested exit\n");
                break;
            }

            // Process request
            resp = process_request(&req);

            // Send response
            if (send(client_fd, resp, sizeof(Response), 0) < 0) {
                perror("send");
            }

            free(resp);
        }

        close(client_fd);
        printf("Client disconnected\n");
    }

    close(server_fd);
    return 0;
}
