#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MAXSIZE 100

int sockfd, newsockfd;

/* Function to check palindrome */
int palindrome(char str[])
{
    int left = 0;
    int right = strlen(str) - 1;

    while (left < right)
    {
        if (str[left] != str[right])
            return 0;   // not palindrome
        left++;
        right--;
    }
    return 1;   // palindrome
}

/* Thread to receive messages from client */
void *recv_thread(void *arg)
{
    char buff[MAXSIZE];
    int n;

    while (1)
    {
        memset(buff, 0, MAXSIZE);
        n = recv(newsockfd, buff, MAXSIZE - 1, 0);

        if (n <= 0)
        {
            printf("Client disconnected.\n");
            exit(0);
        }

        printf("\nClient sent: %s\n", buff);

        char result[MAXSIZE];

        if (strncmp(buff, "stop", 4) == 0)
        {
            printf("Chat ended by client.\n");
            strcpy(result, "Chat ended by server.");
            send(newsockfd, result, strlen(result), 0);
            exit(0);
        }

        if (palindrome(buff))
        {
            printf("Result: Palindrome\n");
            strcpy(result, "Palindrome");
        }
        else
        {
            printf("Result: Not a Palindrome\n");
            strcpy(result, "Not a Palindrome");
        }

        // Send result back to client
        send(newsockfd, result, strlen(result), 0);
    }
}

/* Thread to send messages to client */
void *send_thread(void *arg)
{
    char buff[MAXSIZE];

    while (1)
    {
        memset(buff, 0, MAXSIZE);
        fgets(buff, MAXSIZE, stdin);
        buff[strcspn(buff, "\n")] = '\0';  // remove newline

        send(newsockfd, buff, strlen(buff), 0);

        if (strncmp(buff, "stop", 4) == 0)
        {
            printf("Chat ended by server.\n");
            exit(0);
        }
    }
}

int main()
{
    struct sockaddr_in serveraddr, clientaddr;
    socklen_t addrlen = sizeof(clientaddr);
    pthread_t tid1, tid2;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        perror("Socket creation failed");
        exit(1);
    }

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(3388);
    serveraddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
    {
        perror("Bind failed");
        close(sockfd);
        exit(1);
    }

    if (listen(sockfd, 1) < 0)
    {
        perror("Listen failed");
        close(sockfd);
        exit(1);
    }

    printf("Waiting for client...\n");
    newsockfd = accept(sockfd, (struct sockaddr *)&clientaddr, &addrlen);
    if (newsockfd < 0)
    {
        perror("Accept failed");
        close(sockfd);
        exit(1);
    }
    printf("Client connected.\n");

    // Create receive and send threads
    pthread_create(&tid1, NULL, recv_thread, NULL);
    pthread_create(&tid2, NULL, send_thread, NULL);

    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);

    close(newsockfd);
    close(sockfd);

    return 0;
}
