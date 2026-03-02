#include <stdio.h>

int main() {
    int arrival_times[] = {1, 2, 3, 5, 6, 8, 11, 12, 15, 16, 19};
    int n = 11;
    int packet_size = 4;
    int bucket_size = 10;
    int output_rate = 1;   // 1 byte per second

    int current_bucket = 0;
    int time = 0;
    int i = 0;

    printf("Leaky Bucket Simulation\n");
    printf("------------------------\n");
    printf("Bucket Size = %d bytes\n", bucket_size);
    printf("Output Rate = %d byte/sec\n\n", output_rate);

    int last_time = arrival_times[n-1] + 5;

    for (time = 1; time <= last_time; time++) {

        // Leak data (1 byte per second)
        if (current_bucket > 0) {
            current_bucket -= output_rate;
            if (current_bucket < 0)
                current_bucket = 0;
        }

        // Check packet arrival
        if (i < n && time == arrival_times[i]) {

            printf("Time %2d sec: Packet arrived (4 bytes)\n", time);

            if (current_bucket + packet_size <= bucket_size) {
                current_bucket += packet_size;
                printf("   -> Conforming (Accepted)\n");
            } else {
                printf("   -> Non-Conforming (Dropped)\n");
            }

            i++;
        }

        printf("   Bucket Content: %d bytes\n\n", current_bucket);
        printf(" Hello\n\n");
        printf("NPACN");
    }

    return 0;
}