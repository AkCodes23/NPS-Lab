# NPS-Lab

Network Programming and Security Lab - C Programming Assignments

This repository contains socket programming assignments covering TCP/UDP communication, concurrent servers, DNS systems, and advanced threading concepts.

## 📋 Contents

- **Lab 1**: Basic TCP/UDP Client-Server Communication
- **Lab 3**: Concurrent Server & Process Management
- **Lab 4**: DNS Query System
- **Lab 5**: Advanced Concurrent & Iterative Servers

## 🚀 Quick Start

### Prerequisites
- GCC compiler
- Linux/Unix environment
- pthread library (for concurrent programs)

### Build All Programs

```bash
./build_all.sh
```

This will compile all programs across all labs.

### Clean Build Artifacts

```bash
./clean_all.sh
```

This removes all compiled binaries.

## 📚 Lab Descriptions

### Lab 1: Basic TCP/UDP Client-Server Communication
Introduction to socket programming with TCP and UDP protocols.

**Programs:**
- `client.c` / `server.c` - Basic TCP echo server and client
- `udp_client.c` / `udp_server.c` - UDP communication
- `Conversation_Client_TCP.c` / `Conversation_Server_TCP.c` - Interactive TCP chat
- `Conversation_Client_UDP.c` / `Conversation_Server_UDP.c` - Interactive UDP chat
- `palin_serv.c` - Palindrome checker server

**Compile & Run:**
```bash
cd "Lab 1"
gcc -o server server.c
gcc -o client client.c

# Terminal 1
./server

# Terminal 2
./client
```

### Lab 3: Concurrent Server & Process Management
Advanced concurrent programming using fork() and threading.

**Programs:**
- `concurrent-server.c` / `concurrent-client.c` - Multi-process concurrent server
- `Q1_server.c` / `Q1_Client.c` - Concurrent phrase assembly
- `Q2_server.c` / `Q2_Client.c` - Multi-client handling
- `Q3_server.c` / `Q3_client.c` - Advanced concurrency
- `AQ1_server.c` / `AQ1_client.c` - Additional exercises

**Compile & Run:**
```bash
cd "Lab 3"
gcc -pthread -o concurrent-server concurrent-server.c
gcc -pthread -o concurrent-client concurrent-client.c

# Terminal 1
./concurrent-server

# Terminal 2
./concurrent-client
```

### Lab 4: DNS Query System
Structured client-server system with custom protocol design.

**Programs:**
- `server.c` / `client.c` - Main DNS-like query system
- `dns_server.c` / `dns_client.c` - DNS protocol implementation
- `common.h` - Shared protocol definitions
- `dns_common.h` - DNS-specific structures

**Features:**
- Custom protocol with magic numbers
- TCP and UDP support
- Student registration database
- Menu-driven interface

**Compile & Run:**
```bash
cd "Lab 4"
gcc -o server server.c
gcc -o client client.c

# Terminal 1
./server tcp 8080

# Terminal 2
./client tcp 127.0.0.1 8080
```

### Lab 5: Advanced Concurrent & Iterative Servers
Comparison of concurrent vs iterative server architectures.

**Programs:**
- `Q1_server.c` - Concurrent server with thread pool
- `Q1_client1.c`, `Q1_client2.c`, `Q1_client_extra.c` - Multiple clients
- `Q2_server_iterative.c` / `Q2_client.c` - Iterative server comparison
- `Makefile` - Build configuration

**Compile & Run:**
```bash
cd "Lab 5"
gcc -pthread -o Q1_server Q1_server.c

# Terminal 1
./Q1_server 8080

# Terminal 2
./Q1_client1 127.0.0.1 8080

# Terminal 3
./Q1_client2 127.0.0.1 8080
```

## ✅ Verification

All programs have been verified for:
- ✅ Compilation without errors
- ✅ Security vulnerabilities (fixed)
- ✅ Code quality issues
- ✅ Basic functional testing

See [VERIFICATION_REPORT.md](VERIFICATION_REPORT.md) for detailed findings and fixes.

## 🔒 Security

**Fixed Issues:**
- Replaced dangerous `gets()` with safe `fgets()`
- Fixed incorrect error checking
- Added proper header includes
- Corrected return types and values

## 📝 Notes

- All programs use localhost (127.0.0.1) for communication
- Default ports are specified in each program
- Some programs require interactive input
- Use Ctrl+C to terminate servers

## 🛠️ Development

### Building Individual Programs

Each lab can be built independently:

```bash
cd "Lab X"
gcc -o program_name source_file.c

# For programs requiring threading
gcc -pthread -o program_name source_file.c
```

### Common Issues

1. **Port already in use**: Wait a few seconds after closing server or use a different port
2. **Permission denied**: Ensure scripts are executable: `chmod +x script.sh`
3. **Connection refused**: Make sure the server is running before starting the client

## 📖 Reference

- Socket Programming: `man socket`, `man bind`, `man listen`, `man accept`
- Threading: `man pthread_create`, `man pthread_mutex`
- Network Functions: `man inet_addr`, `man htons`

## 📄 License

Educational project for Network Programming and Security Lab coursework.
