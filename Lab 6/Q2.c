#include <stdio.h>
#include <string.h>

void xorOperation(char *temp, char *gen, int genLen) {
    for (int i = 0; i < genLen; i++)
        temp[i] = (temp[i] == gen[i]) ? '0' : '1';
}

void crc(char *data, char *gen, char *remainder) {
    int dataLen = strlen(data);
    int genLen = strlen(gen);
    char temp[50];

    strcpy(temp, data);

    for (int i = 0; i <= dataLen - genLen; i++) {
        if (temp[i] == '1')
            xorOperation(&temp[i], gen, genLen);
    }

    strncpy(remainder, &temp[dataLen - genLen + 1], genLen - 1);
    remainder[genLen - 1] = '\0';
}

int main() {
    char data[50], gen[20], remainder[20], received[50];

    printf("Enter data bits: ");
    scanf("%s", data);

    printf("Enter generator polynomial: ");
    scanf("%s", gen);

    int genLen = strlen(gen);
    for (int i = 0; i < genLen - 1; i++)
        strcat(data, "0");

    crc(data, gen, remainder);

    printf("CRC bits: %s\n", remainder);
    printf("Transmitted Data: %s%s\n", data, remainder);

    printf("Enter received data: ");
    scanf("%s", received);

    crc(received, gen, remainder);

    int error = 0;
    for (int i = 0; i < strlen(remainder); i++)
        if (remainder[i] == '1')
            error = 1;

    if (error)
        printf("Error Detected\n");
    else
        printf("No Error Detected\n");

    return 0;
}
