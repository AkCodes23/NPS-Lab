#include <stdio.h>
#include <math.h>

int main() {
    int data[20], hamming[30], received[30];
    int i, j, k, n, p = 0, errorPos = 0;

    printf("Enter number of data bits: ");
    scanf("%d", &n);

    printf("Enter data bits:\n");
    for (i = 0; i < n; i++)
        scanf("%d", &data[i]);

    // Calculate parity bits
    while ((1 << p) < (n + p + 1))
        p++;

    j = 0;
    k = 0;
    for (i = 1; i <= n + p; i++) {
        if ((i & (i - 1)) == 0)
            hamming[i] = 0;
        else
            hamming[i] = data[j++];
    }

    // Calculate parity values
    for (i = 0; i < p; i++) {
        int parity = 0;
        for (j = 1; j <= n + p; j++)
            if (j & (1 << i))
                parity ^= hamming[j];
        hamming[1 << i] = parity;
    }

    printf("Transmitted Hamming Code:\n");
    for (i = 1; i <= n + p; i++)
        printf("%d ", hamming[i]);

    printf("\nEnter received data:\n");
    for (i = 1; i <= n + p; i++)
        scanf("%d", &received[i]);

    // Error detection
    for (i = 0; i < p; i++) {
        int parity = 0;
        for (j = 1; j <= n + p; j++)
            if (j & (1 << i))
                parity ^= received[j];
        if (parity)
            errorPos += (1 << i);
    }

    if (errorPos == 0)
        printf("No Error Detected\n");
    else {
        printf("Error at position: %d\n", errorPos);
        received[errorPos] ^= 1;
        printf("Corrected Data:\n");
        for (i = 1; i <= n + p; i++)
            printf("%d ", received[i]);
    }

    return 0;
}
