# Lab 3: Concurrent Server & Process Management

This lab focuses on concurrent programming using process creation (`fork()`) and threading to handle multiple clients simultaneously.

## Programs

### 1. Concurrent Server and Client
Multi-process server that handles clients using `fork()`.

**Files:**
- `concurrent-server.c` - Server that creates child processes for each client
- `concurrent-client.c` - Client that connects to the concurrent server

**How to Run:**
```bash
# Compile
gcc -pthread -o concurrent-server concurrent-server.c
gcc -pthread -o concurrent-client concurrent-client.c

# Terminal 1: Start server
./concurrent-server

# Terminal 2: Start first client
./concurrent-client

# Terminal 3: Start second client (optional)
./concurrent-client
```

**Port:** 4444

**How it works:**
- Server creates a new child process for each client connection
- Parent process continues to accept new connections
- Child processes handle client communication independently

---

### 2. Q1: Concurrent Phrase Assembly
Server coordinates multiple clients to build a complete phrase using fork().

**Files:**
- `Q1_server.c` - Concurrent server with parent/child process communication
- `Q1_Client.c` - Client for Q1 server

**How to Run:**
```bash
# Compile
gcc -pthread -o Q1_server Q1_server.c
gcc -pthread -o Q1_Client Q1_Client.c

# Terminal 1: Start server
./Q1_server

# Terminal 2: Start client
./Q1_Client
```

**Port:** 8080

**Features:**
- Parent process sends messages to client
- Child process receives messages from client
- Bidirectional communication using separate processes

---

### 3. Q2: Multi-Client Server
Server that handles multiple clients concurrently.

**Files:**
- `Q2_server.c` - Multi-client concurrent server
- `Q2_Client.c` - Client for Q2 server

**How to Run:**
```bash
# Compile
gcc -pthread -o Q2_server Q2_server.c
gcc -pthread -o Q2_Client Q2_Client.c

# Terminal 1: Start server
./Q2_server

# Terminal 2+: Start multiple clients
./Q2_Client
```

---

### 4. Q3: Advanced Concurrency
Advanced concurrent server implementation.

**Files:**
- `Q3_server.c` - Advanced concurrent server
- `Q3_client.c` - Client for Q3 server

**How to Run:**
```bash
# Compile
gcc -pthread -o Q3_server Q3_server.c
gcc -pthread -o Q3_client Q3_client.c

# Terminal 1: Start server
./Q3_server

# Terminal 2: Start client
./Q3_client
```

---

### 5. AQ1: Additional Exercise
Additional concurrent programming exercise.

**Files:**
- `AQ1_server.c` - Additional exercise server
- `AQ1_client.c` - Client for AQ1 server

**How to Run:**
```bash
# Compile
gcc -pthread -o AQ1_server AQ1_server.c
gcc -pthread -o AQ1_client AQ1_client.c

# Terminal 1: Start server
./AQ1_server

# Terminal 2: Start client
./AQ1_client
```

---

## Learning Objectives

1. **Process Creation**: Using `fork()` to create child processes
2. **Concurrent Programming**: Handling multiple clients simultaneously
3. **Process Communication**: Parent-child process communication
4. **Process Management**: Understanding PIDs, parent/child relationships
5. **Resource Sharing**: Managing shared resources between processes

## Key Concepts

### Process Creation with fork()
```c
pid_t pid = fork();

if (pid > 0) {
    // Parent process
    printf("Parent PID: %d\n", getpid());
} else if (pid == 0) {
    // Child process
    printf("Child PID: %d\n", getpid());
} else {
    // Error
    perror("fork failed");
}
```

### Concurrent vs Sequential
- **Sequential Server**: Handles one client at a time
- **Concurrent Server**: Handles multiple clients simultaneously using:
  - Multiple processes (fork)
  - Multiple threads (pthread)
  - Event-driven (select/poll/epoll)

### Process States
- Running: Currently executing
- Waiting: Waiting for I/O or child process
- Zombie: Terminated but not yet reaped by parent

## Important Functions

- `fork()` - Create a new process
- `getpid()` - Get current process ID
- `getppid()` - Get parent process ID
- `wait()` / `waitpid()` - Wait for child process to terminate
- `exit()` - Terminate process

## Common Issues

1. **Zombie Processes**: Parent doesn't call `wait()` on terminated children
   - Solution: Use `wait()` or `waitpid()` in parent process
   
2. **Port Already in Use**: Previous server instance still running
   - Solution: Kill previous process or use `SO_REUSEADDR`

3. **Too Many Processes**: Fork bomb risk
   - Solution: Limit maximum concurrent connections

4. **Shared Resource Conflicts**: Multiple processes accessing same resource
   - Solution: Use proper synchronization mechanisms

## Threading Notes

Some programs use the `-pthread` flag for threading support. This is for potential pthread usage in concurrent operations.

### Process vs Thread
- **Process**: Independent execution context with own memory space
- **Thread**: Lightweight execution context sharing memory space

## Testing Multiple Clients

To test concurrent handling:
```bash
# Terminal 1: Server
./concurrent-server

# Terminals 2-N: Launch multiple clients
for i in {1..5}; do
    ./concurrent-client &
done
```

## Performance Considerations

1. **Process Creation Overhead**: fork() is expensive
2. **Memory Usage**: Each process has its own memory space
3. **Context Switching**: OS overhead switching between processes
4. **Maximum Processes**: System has limits on number of processes

## Debugging

Check process information:
```bash
# List all processes
ps aux | grep server

# List process tree
pstree -p

# Monitor system calls
strace ./concurrent-server
```

## References

- `man 2 fork` - Create a child process
- `man 2 getpid` - Get process ID
- `man 2 wait` - Wait for process termination
- `man 7 signal` - Signal handling
- Beej's Guide to Network Programming
