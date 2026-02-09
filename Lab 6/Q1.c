#include <stdio.h>

int main() {
    int data[20], received[20];
    int n, i, choice, count = 0;

    printf("Enter number of bits: ");
    scanf("%d", &n);

    printf("Enter data bits:\n");
    for (i = 0; i < n; i++)
        scanf("%d", &data[i]);

    printf("Choose Parity:\n1. Even Parity\n2. Odd Parity\n");
    scanf("%d", &choice);

    // Sender side
    for (i = 0; i < n; i++)
        if (data[i] == 1)
            count++;

    int parity;
    if (choice == 1)
        parity = (count % 2 == 0) ? 0 : 1;
    else
        parity = (count % 2 == 0) ? 1 : 0;

    printf("Transmitted Data: ");
    for (i = 0; i < n; i++)
        printf("%d", data[i]);
    printf("%d\n", parity);

    // Receiver side
    printf("Enter received data including parity bit:\n");
    for (i = 0; i <= n; i++)
        scanf("%d", &received[i]);

    count = 0;
    for (i = 0; i <= n; i++)
        if (received[i] == 1)
            count++;

    if ((choice == 1 && count % 2 == 0) ||
        (choice == 2 && count % 2 != 0))
        printf("No Error Detected\n");
    else
        printf("Error Detected\n");

    return 0;
}
