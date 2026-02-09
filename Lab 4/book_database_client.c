#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>

#define PORT 13000
#define SERVER_IP "127.0.0.1"
#define MAX_BOOKS 100
#define MAX_TITLE_LEN 100
#define MAX_AUTHOR_LEN 50
#define MAX_PUBLISHER_LEN 50
#define BUFFER_SIZE 4096

// Book structure
typedef struct {
    int accession_number;
    char title[MAX_TITLE_LEN];
    char author[MAX_AUTHOR_LEN];
    int total_pages;
    char publisher[MAX_PUBLISHER_LEN];
} Book;

// Request structure for client-server communication
typedef struct {
    int operation;  // 1: Insert, 2: Delete, 3: Display All, 4: Search, 5: Exit
    Book book;
    char search_query[MAX_AUTHOR_LEN];  // For search operations
} Request;

// Response structure
typedef struct {
    int status;  // 1: Success, 0: Failure
    char message[256];
    int book_count;
    Book books[MAX_BOOKS];
} Response;

void display_menu() {
    printf("\n========== Book Database Menu ==========\n");
    printf("1. Insert a new book\n");
    printf("2. Delete a book\n");
    printf("3. Display all books\n");
    printf("4. Search a book (by title or author)\n");
    printf("5. Exit\n");
    printf("========================================\n");
}

void display_book(Book *book) {
    printf("\n┌─────────────────────────────────────────────┐\n");
    printf("│ Accession #: %d\n", book->accession_number);
    printf("│ Title      : %-35s │\n", book->title);
    printf("│ Author     : %-35s │\n", book->author);
    printf("│ Publisher  : %-35s │\n", book->publisher);
    printf("│ Pages      : %d\n", book->total_pages);
    printf("└─────────────────────────────────────────────┘\n");
}

int main() {
    int sock;
    struct sockaddr_in server_addr;
    Request req;
    Response resp;
    int choice;

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

    printf("Connected to Book Database Server %s:%d\n\n", SERVER_IP, PORT);

    while (1) {
        display_menu();
        printf("Enter your choice: ");
        scanf("%d", &choice);
        getchar();  // Consume newline after number input

        memset(&req, 0, sizeof(Request));
        req.operation = choice;

        if (choice < 1 || choice > 5) {
            printf("Invalid choice! Please enter a valid option.\n");
            continue;
        }

        if (choice == 1) {  // Insert Book
            printf("\n--- Insert New Book ---\n");
            printf("Enter accession number: ");
            scanf("%d", &req.book.accession_number);
            getchar();

            printf("Enter title: ");
            fgets(req.book.title, MAX_TITLE_LEN, stdin);
            req.book.title[strcspn(req.book.title, "\n")] = '\0';

            printf("Enter author: ");
            fgets(req.book.author, MAX_AUTHOR_LEN, stdin);
            req.book.author[strcspn(req.book.author, "\n")] = '\0';

            printf("Enter publisher: ");
            fgets(req.book.publisher, MAX_PUBLISHER_LEN, stdin);
            req.book.publisher[strcspn(req.book.publisher, "\n")] = '\0';

            printf("Enter total pages: ");
            scanf("%d", &req.book.total_pages);
            getchar();

            // Send request to server
            if (send(sock, &req, sizeof(Request), 0) < 0) {
                perror("send");
                break;
            }

            // Receive response from server
            if (recv(sock, &resp, sizeof(Response), 0) <= 0) {
                printf("Connection closed by server\n");
                break;
            }

            if (resp.status == 1) {
                printf("\n✓ SUCCESS: %s\n", resp.message);
            } else {
                printf("\n✗ FAILURE: %s\n", resp.message);
            }

        } else if (choice == 2) {  // Delete Book
            printf("\n--- Delete Book ---\n");
            printf("Enter accession number of the book to delete: ");
            scanf("%d", &req.book.accession_number);
            getchar();

            // Send request to server
            if (send(sock, &req, sizeof(Request), 0) < 0) {
                perror("send");
                break;
            }

            // Receive response from server
            if (recv(sock, &resp, sizeof(Response), 0) <= 0) {
                printf("Connection closed by server\n");
                break;
            }

            if (resp.status == 1) {
                printf("\n✓ SUCCESS: %s\n", resp.message);
            } else {
                printf("\n✗ FAILURE: %s\n", resp.message);
            }

        } else if (choice == 3) {  // Display All Books
            printf("\n--- Display All Books ---\n");

            // Send request to server
            if (send(sock, &req, sizeof(Request), 0) < 0) {
                perror("send");
                break;
            }

            // Receive response from server
            if (recv(sock, &resp, sizeof(Response), 0) <= 0) {
                printf("Connection closed by server\n");
                break;
            }

            if (resp.book_count > 0) {
                printf("\n%s\n", resp.message);
                for (int i = 0; i < resp.book_count; i++) {
                    display_book(&resp.books[i]);
                }
            } else {
                printf("\nNo books in the database.\n");
            }

        } else if (choice == 4) {  // Search Book
            printf("\n--- Search Book ---\n");
            printf("Enter search term (author name or book title): ");
            fgets(req.search_query, MAX_AUTHOR_LEN, stdin);
            req.search_query[strcspn(req.search_query, "\n")] = '\0';

            // Send request to server
            if (send(sock, &req, sizeof(Request), 0) < 0) {
                perror("send");
                break;
            }

            // Receive response from server
            if (recv(sock, &resp, sizeof(Response), 0) <= 0) {
                printf("Connection closed by server\n");
                break;
            }

            printf("\n%s\n", resp.message);
            if (resp.book_count > 0) {
                for (int i = 0; i < resp.book_count; i++) {
                    display_book(&resp.books[i]);
                }
            }

        } else if (choice == 5) {  // Exit
            printf("\n--- Exiting ---\n");
            
            // Send exit request to server
            if (send(sock, &req, sizeof(Request), 0) < 0) {
                perror("send");
            }

            printf("Thank you for using the Book Database System!\n");
            break;
        }
    }

    close(sock);
    return 0;
}
