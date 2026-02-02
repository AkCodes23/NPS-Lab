# Lab 1: Basic TCP/UDP Client-Server Communication

This lab introduces fundamental socket programming concepts using both TCP and UDP protocols.

## Programs

### 1. TCP Echo Server and Client
Basic TCP communication where the server echoes back messages from the client.

**Files:**
- `client.c` - TCP client that connects to server and sends messages
- `server.c` - TCP server that accepts connections and echoes messages

**How to Run:**
```bash
# Compile
gcc -o server server.c
gcc -o client client.c

# Terminal 1: Start server
./server

# Terminal 2: Run client
./client
# Enter your message when prompted
```

**Port:** 3388

---

### 2. UDP Client and Server
UDP-based communication with character-by-character transmission.

**Files:**
- `udp_client.c` - UDP client
- `udp_server.c` - UDP server

**How to Run:**
```bash
# Compile
gcc -o udp_client udp_client.c
gcc -o udp_server udp_server.c

# Terminal 1: Start UDP server
./udp_server

# Terminal 2: Run UDP client
./udp_client
```

**Ports:** 
- Server: 3200
- Client: 3201

---

### 3. TCP Conversation Programs
Interactive bidirectional TCP chat between client and server.

**Files:**
- `Conversation_Client_TCP.c` - Interactive TCP client
- `Conversation_Server_TCP.c` - Interactive TCP server

**How to Run:**
```bash
# Compile
gcc -o conv_tcp_server Conversation_Server_TCP.c
gcc -o conv_tcp_client Conversation_Client_TCP.c

# Terminal 1: Start server
./conv_tcp_server

# Terminal 2: Start client
./conv_tcp_client
```

**Features:**
- Bidirectional communication
- Type "stop" to end conversation
- Interactive message exchange

**Port:** 3388

---

### 4. UDP Conversation Programs
Interactive bidirectional UDP chat.

**Files:**
- `Conversation_Client_UDP.c` - Interactive UDP client
- `Conversation_Server_UDP.c` - Interactive UDP server

**How to Run:**
```bash
# Compile
gcc -o conv_udp_server Conversation_Server_UDP.c
gcc -o conv_udp_client Conversation_Client_UDP.c

# Terminal 1: Start server
./conv_udp_server

# Terminal 2: Start client
./conv_udp_client
```

---

### 5. Palindrome Server
Server that checks if strings are palindromes.

**File:** `palin_serv.c`

**How to Run:**
```bash
# Compile
gcc -o palin_serv palin_serv.c

# Run
./palin_serv
```

---

## Learning Objectives

1. **Socket Creation**: Understanding `socket()`, `bind()`, `listen()`, `accept()`, `connect()`
2. **TCP vs UDP**: Differences between connection-oriented and connectionless protocols
3. **Client-Server Model**: Basic architecture of networked applications
4. **Data Transmission**: Using `send()`, `recv()`, `sendto()`, `recvfrom()`
5. **Network Byte Order**: Using `htons()`, `htonl()`, `inet_addr()`

## Key Concepts

### TCP (Transmission Control Protocol)
- Connection-oriented
- Reliable delivery
- Stream-based
- Uses `connect()`, `send()`, `recv()`

### UDP (User Datagram Protocol)
- Connectionless
- No delivery guarantee
- Message-based
- Uses `sendto()`, `recvfrom()`

## Common Issues

1. **Address already in use**: The port is still bound from a previous run. Wait a few seconds or use `SO_REUSEADDR` socket option.
2. **Connection refused**: Server is not running. Start the server before the client.
3. **Segmentation fault**: Check buffer sizes and null terminators.

## Security Notes

⚠️ **Security Fix Applied**: The dangerous `gets()` function has been replaced with safe `fgets()` in UDP programs to prevent buffer overflow vulnerabilities.

## Network Configuration

All programs use:
- **IP Address**: 127.0.0.1 (localhost/loopback)
- **Protocol Family**: AF_INET (IPv4)
- **Ports**: As specified in each program (typically 3200, 3201, 3388)

## Testing

To test if a server is listening on a port:
```bash
netstat -tuln | grep <port_number>
```

To test TCP connectivity:
```bash
telnet 127.0.0.1 <port_number>
```

## References

- `man 2 socket` - Socket system call
- `man 2 bind` - Bind socket to address
- `man 2 listen` - Listen for connections
- `man 2 accept` - Accept connection
- `man 2 connect` - Connect to server
- `man 2 send` - Send data
- `man 2 recv` - Receive data
