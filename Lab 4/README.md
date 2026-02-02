# Lab 4: DNS Query System

This lab implements a DNS-like client-server system with a custom protocol for querying student registration data.

## Overview

A structured request-response protocol system that allows clients to query student information (registration number, name, subject codes) from a server database.

## Architecture

```
Client → [Request Protocol] → Server (with Database)
       ← [Response Protocol] ←
```

## Programs

### 1. Main Client-Server System

**Files:**
- `server.c` - Main server with student database
- `client.c` - Interactive menu-driven client
- `common.h` - Shared protocol definitions and structures

**How to Run:**
```bash
# Compile
gcc -o server server.c
gcc -o client client.c

# Terminal 1: Start server (TCP mode, port 8080)
./server tcp 8080

# Or UDP mode
./server udp 8080

# Terminal 2: Run client
./client tcp 127.0.0.1 8080

# Or UDP mode
./client udp 127.0.0.1 8080
```

**Features:**
- Menu-driven interface
- Query by:
  1. Registration Number
  2. Student Name
  3. Subject Code
- Both TCP and UDP support
- Structured request/response protocol

---

### 2. DNS Client-Server

**Files:**
- `dns_server.c` - DNS-like server
- `dns_client.c` - DNS client
- `dns_common.h` - DNS protocol definitions

**How to Run:**
```bash
# Compile
gcc -o dns_server dns_server.c
gcc -o dns_client dns_client.c

# Terminal 1: Start DNS server
./dns_server 9090

# Terminal 2: Run DNS client
./dns_client 127.0.0.1 9090
```

---

## Protocol Design

### Request Structure
```c
typedef struct {
    uint32_t magic;             // APP_MAGIC (0x4C423431 = "LB41")
    uint32_t option;            // Query type (1=regno, 2=name, 3=subject)
    char regno[MAX_REGNO];      // Registration number
    char name[MAX_NAME];        // Student name
    char subject[MAX_SUBJECT];  // Subject code
} request_t;
```

### Response Structure
```c
typedef struct {
    uint32_t magic;             // APP_MAGIC
    int32_t status;             // 0 = success, nonzero = error
    int32_t child_pid;          // Server process ID
    char message[MAX_MESSAGE];  // Response data or error message
} response_t;
```

### Magic Number
- **Value**: `0x4C423431` (ASCII: "LB41")
- **Purpose**: Protocol identification and validation

## Sample Student Database

The server contains pre-populated student data:

```c
Student 1:
- Regno: 23CS001
- Name: Asha
- Address: 12, MG Road, Bengaluru
- Dept: CSE
- Semester: 4
- Section: A
- Courses: CS201, CS202, MA201

Student 2:
- Regno: 23EC014
- Name: Rahul
- Address: 44, Lake View, Chennai
- Dept: ECE
- Semester: 3
- Section: B
- Courses: EC210, EC211, MA201
```

### Subject Marks
- CS201: 88
- CS202: 79
- MA201: 91
- EC210: 84
- EC211: 77

## Query Examples

### Query by Registration Number
```
Enter option: 1
Enter Registration Number: 23CS001

Response:
Student: Asha
Address: 12, MG Road, Bengaluru
Department: CSE
Semester: 4, Section: A
Courses: CS201, CS202, MA201
```

### Query by Name
```
Enter option: 2
Enter Name of Student: Rahul

Response:
Regno: 23EC014
Student: Rahul
Department: ECE
```

### Query by Subject Code
```
Enter option: 3
Enter Subject Code: CS201

Response:
Subject: CS201
Marks: 88
```

## Protocol Features

### 1. Safe Data Transmission
- Uses `send_all()` and `recv_all()` helpers
- Handles partial sends/receives
- Proper error handling

### 2. Network Byte Order
- Uses `htonl()` and `ntohl()` for 32-bit values
- Ensures cross-platform compatibility

### 3. Field Width Protection
```c
sscanf(line, "%255s %63s", domain, ip);  // Prevents buffer overflow
```

## Learning Objectives

1. **Protocol Design**: Creating custom application protocols
2. **Data Structures**: Using structs for network communication
3. **Serialization**: Converting data structures for network transmission
4. **Client-Server Communication**: Request-response patterns
5. **Error Handling**: Proper error checking and reporting
6. **Process Management**: Using fork() for concurrent client handling

## Key Concepts

### Application Protocol
A set of rules defining:
- Message format
- Message types
- Exchange patterns
- Error handling

### Magic Numbers
Used for:
- Protocol identification
- Version checking
- Validation
- Preventing accidental data interpretation

### TCP vs UDP in this Lab
**TCP Mode:**
- Reliable delivery
- Connection-oriented
- Good for critical data

**UDP Mode:**
- Faster
- No connection overhead
- Good for queries where occasional loss is acceptable

## Security Considerations

✅ **Good Practices Used:**
1. Fixed-size buffers with length checks
2. Safe string functions (`snprintf()`)
3. Input validation with `sscanf()` field widths
4. Error checking on all system calls

## Testing

### Test Complete Flow
```bash
# Start server
./server tcp 8080

# In another terminal, test queries
./client tcp 127.0.0.1 8080
# Try each menu option (1, 2, 3)
```

### Test Error Handling
```bash
# Query non-existent student
Enter Registration Number: 99XX999
# Should get error message
```

### Test Concurrent Clients
```bash
# Terminal 1: Server
./server tcp 8080

# Terminals 2-N: Multiple clients
./client tcp 127.0.0.1 8080 &
./client tcp 127.0.0.1 8080 &
```

## Advanced Features

### Process-based Concurrency
Server uses `fork()` to handle multiple clients:
```c
if (fork() == 0) {
    // Child process handles client
    handle_client(client_fd);
    exit(0);
}
// Parent continues accepting connections
```

### Graceful Shutdown
Server handles signals for clean shutdown:
- SIGINT (Ctrl+C)
- SIGTERM
- Cleanup of child processes

## Common Issues

1. **Invalid Magic Number**: Client/server protocol mismatch
2. **Timeout**: Server not responding (check if running)
3. **Connection Refused**: Server not listening on specified port
4. **Partial Data**: Network interruption (handled by send_all/recv_all)

## References

- `man 3 htonl` - Network byte order conversion
- `man 2 fork` - Process creation
- `man 3 sscanf` - Formatted input scanning
- `man 7 socket` - Socket programming overview
- RFC 1035 - Domain Names (DNS inspiration)
