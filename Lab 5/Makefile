CC=gcc
CFLAGS=-Wall -Wextra -O2 -pthread

all: Q1_server Q1_client1 Q1_client2 Q1_client_extra Q2_server_iterative Q2_client

Q1_server: Q1_server.c
	$(CC) $(CFLAGS) -o $@ $<

Q1_client1: Q1_client1.c
	$(CC) $(CFLAGS) -o $@ $<

Q1_client2: Q1_client2.c
	$(CC) $(CFLAGS) -o $@ $<

Q1_client_extra: Q1_client_extra.c
	$(CC) $(CFLAGS) -o $@ $<

Q2_server_iterative: Q2_server_iterative.c
	$(CC) $(CFLAGS) -o $@ $<

Q2_client: Q2_client.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f Q1_server Q1_client1 Q1_client2 Q1_client_extra Q2_server_iterative Q2_client
