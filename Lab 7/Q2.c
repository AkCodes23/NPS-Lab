#include <stdio.h>

int main() {

    float r = 10.0;          // token generation rate (KB per sec)
    float bucket_size = 50;  // bucket capacity (KB)
    float tokens = 50;       // initially full
    float packet_size = 15;  // packet size (KB)
    float time = 0.0;
    float end_time = 5.0;    // simulate 5 seconds
    float interval = 0.5;    // packet every 0.5 sec

    int queued_packets = 0;

    printf("TOKEN BUCKET SIMULATION\n");
    printf("-------------------------\n");

    for(time = 0.5; time <= end_time; time += interval) {

        // Add tokens generated in last 0.5 seconds
        tokens += r * interval;

        if(tokens > bucket_size)
            tokens = bucket_size;

        printf("\nTime = %.1f sec\n", time);
        printf("Tokens available before sending: %.2f KB\n", tokens);

        if(tokens >= packet_size) {
            tokens -= packet_size;
            printf("Packet transmitted (15 KB)\n");
        }
        else {
            queued_packets++;
            printf("Packet queued (Not enough tokens)\n");
        }

        printf("Tokens left: %.2f KB\n", tokens);
    }

    printf("\nTotal queued packets: %d\n", queued_packets);

    return 0;
}