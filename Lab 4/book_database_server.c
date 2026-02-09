#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 13000
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

// Global book database
Book database[MAX_BOOKS];
int total_books = 0;

// Function to insert a new book
int insert_book(Book *book) {
    // Check if book with same accession number exists
    for (int i = 0; i < total_books; i++) {
        if (database[i].accession_number == book->accession_number) {
            return 0;  // Failure: Accession number already exists
        }
    }

    if (total_books >= MAX_BOOKS) {
        return 0;  // Failure: Database is full
    }

    // Add book to database
    database[total_books] = *book;
    total_books++;
    return 1;  // Success
}

// Function to delete a book by accession number
int delete_book(int accession_number) {
    for (int i = 0; i < total_books; i++) {
        if (database[i].accession_number == accession_number) {
            // Shift remaining books
            for (int j = i; j < total_books - 1; j++) {
                database[j] = database[j + 1];
            }
            total_books--;
            return 1;  // Success
        }
    }
    return 0;  // Failure: Book not found
}

// Function to get all books
int get_all_books(Book *books) {
    for (int i = 0; i < total_books; i++) {
        books[i] = database[i];
    }
    return total_books;
}

// Function to search books by title or author
int search_books(char *query, Book *results) {
    int count = 0;
    
    // Search by author name first (case-insensitive)
    for (int i = 0; i < total_books; i++) {
        if (strcasestr(database[i].author, query) != NULL) {
            results[count] = database[i];
            count++;
        }
    }
    
    // If no results by author, search by title
    if (count == 0) {
        for (int i = 0; i < total_books; i++) {
            if (strcasestr(database[i].title, query) != NULL) {
                results[count] = database[i];
                count++;
            }
        }
    }
    
    return count;
}

// Function to load database from file
void load_database_from_file() {
    FILE *file = fopen("books.db", "rb");
    if (file == NULL) {
        // File doesn't exist, start with empty database
        total_books = 0;
        return;
    }

    fread(&total_books, sizeof(int), 1, file);
    fread(database, sizeof(Book), total_books, file);
    fclose();

    printf("Database loaded: %d books\n", total_books);
}

// Function to save database to file
void save_database_to_file() {
    FILE *file = fopen("books.db", "wb");
    if (file == NULL) {
        printf("Error: Could not save database to file\n");
        return;
    }

    fwrite(&total_books, sizeof(int), 1, file);
    fwrite(database, sizeof(Book), total_books, file);
    fclose();

    printf("Database saved: %d books\n", total_books);
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len;
    Request req;
    Response resp;

    // Load database from file
    load_database_from_file();

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
    if (listen(server_fd, 5) < 0) {
        perror("listen");
        exit(1);
    }

    printf("=== Book Database Server ===\n");
    printf("Server listening on port %d...\n", PORT);

    while (1) {
        // Accept connection
        client_addr_len = sizeof(client_addr);
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_fd < 0) {
            perror("accept");
            continue;
        }

        printf("\nClient connected: %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        // Process requests from client
        while (1) {
            // Receive request
            if (recv(client_fd, &req, sizeof(Request), 0) <= 0) {
                break;  // Client disconnected
            }

            memset(&resp, 0, sizeof(Response));

            printf("Received operation: %d\n", req.operation);

            if (req.operation == 1) {  // Insert
                printf("  Inserting book: %s by %s\n", req.book.title, req.book.author);
                if (insert_book(&req.book)) {
                    resp.status = 1;
                    sprintf(resp.message, "Book inserted successfully! (Accession #%d)", req.book.accession_number);
                    save_database_to_file();
                } else {
                    resp.status = 0;
                    sprintf(resp.message, "Error: Could not insert book. Accession number may already exist or database is full.");
                }

            } else if (req.operation == 2) {  // Delete
                printf("  Deleting book with accession number: %d\n", req.book.accession_number);
                if (delete_book(req.book.accession_number)) {
                    resp.status = 1;
                    sprintf(resp.message, "Book deleted successfully!");
                    save_database_to_file();
                } else {
                    resp.status = 0;
                    sprintf(resp.message, "Error: Book not found!");
                }

            } else if (req.operation == 3) {  // Display All
                printf("  Retrieving all books (%d total)\n", total_books);
                resp.book_count = get_all_books(resp.books);
                resp.status = 1;
                sprintf(resp.message, "Retrieved %d books from database", resp.book_count);

            } else if (req.operation == 4) {  // Search
                printf("  Searching for: %s\n", req.search_query);
                resp.book_count = search_books(req.search_query, resp.books);
                resp.status = 1;
                if (resp.book_count > 0) {
                    sprintf(resp.message, "Found %d book(s) matching '%s'", resp.book_count, req.search_query);
                } else {
                    sprintf(resp.message, "No books found matching '%s'", req.search_query);
                }

            } else if (req.operation == 5) {  // Exit
                printf("  Client requested exit\n");
                save_database_to_file();
                close(client_fd);
                break;
            }

            // Send response
            if (send(client_fd, &resp, sizeof(Response), 0) < 0) {
                perror("send");
                break;
            }
        }

        printf("Client disconnected\n");
    }

    close(server_fd);
    save_database_to_file();
    return 0;
}
