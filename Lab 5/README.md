# Lab 5: Advanced Concurrent & Iterative Servers

This lab explores advanced server architectures, comparing concurrent (multi-threaded) and iterative (sequential) approaches for handling multiple clients.

## Overview

This lab demonstrates:
- Thread-based concurrent server architecture
- Client synchronization and coordination
- Iterative server implementation
- Performance trade-offs between different approaches

## Programs

### 1. Q1: Concurrent Thread Pool Server

A sophisticated multi-threaded server that coordinates multiple clients to build a complete phrase.

**Files:**
- `Q1_server.c` - Concurrent server with pthread mutex synchronization
- `Q1_client1.c` - First client (sends "National Institute of")
- `Q1_client2.c` - Second client (sends "Technology Karnataka")
- `Q1_client_extra.c` - Additional client for testing

**How to Run:**
```bash
# Compile
gcc -pthread -o Q1_server Q1_server.c
gcc -o Q1_client1 Q1_client1.c
gcc -o Q1_client2 Q1_client2.c
gcc -o Q1_client_extra Q1_client_extra.c

# Terminal 1: Start server
./Q1_server 8080

# Terminal 2: Start first client
./Q1_client1 127.0.0.1 8080

# Terminal 3: Start second client
./Q1_client2 127.0.0.1 8080

# Optional: Start additional client
./Q1_client_extra 127.0.0.1 8080
```

**Port:** 8080 (or specified)

**How it works:**
1. Server waits for two clients to connect
2. Each client submits a keyword
3. Server combines keywords into complete phrase
4. Server broadcasts final phrase to all clients
5. Phrase: "National Institute of Technology Karnataka"

**Features:**
- Thread-safe client management
- Mutex-based synchronization
- Graceful shutdown handling
- Support for multiple concurrent clients

---

### 2. Q2: Iterative vs Concurrent Comparison

Compare iterative (sequential) and concurrent server architectures.

**Files:**
- `Q2_server_iterative.c` - Iterative server (handles one client at a time)
- `Q2_client.c` - Client for testing both servers

**How to Run:**
```bash
# Compile
gcc -o Q2_server_iterative Q2_server_iterative.c
gcc -o Q2_client Q2_client.c

# Test Iterative Server
# Terminal 1: Start iterative server
./Q2_server_iterative

# Terminal 2: First client (will be served)
./Q2_client

# Terminal 3: Second client (will wait until first completes)
./Q2_client
```

**Key Difference:**
- **Iterative**: Handles clients sequentially (one at a time)
- **Concurrent**: Handles multiple clients simultaneously (using threads/processes)

---

## Build System

**Makefile** is provided for convenient building:
```bash
# Build all programs
make

# Clean binaries
make clean

# Build specific target
make Q1_server
```

---

## Learning Objectives

1. **Threading**: Using pthread library for concurrent execution
2. **Synchronization**: Mutex locks for shared resource protection
3. **Thread Safety**: Avoiding race conditions
4. **Client Coordination**: Synchronizing multiple clients
5. **Architecture Comparison**: Understanding trade-offs

## Key Concepts

### Concurrent Server Architecture

```c
// For each client connection
pthread_t thread;
pthread_create(&thread, NULL, client_handler, &client_fd);
pthread_detach(thread);
```

**Advantages:**
- ✅ Multiple clients served simultaneously
- ✅ Better resource utilization
- ✅ Responsive to all clients
- ✅ Scalable

**Disadvantages:**
- ❌ Complex synchronization needed
- ❌ Race condition risks
- ❌ Higher memory usage per thread
- ❌ Debugging complexity

### Iterative Server Architecture

```c
// Sequential client handling
while (1) {
    client_fd = accept(server_fd, ...);
    handle_client(client_fd);  // Blocks until done
    close(client_fd);
}
```

**Advantages:**
- ✅ Simple implementation
- ✅ No synchronization needed
- ✅ Easy to debug
- ✅ Lower memory overhead

**Disadvantages:**
- ❌ Only one client at a time
- ❌ Poor resource utilization
- ❌ Other clients must wait
- ❌ Not scalable

## Thread Synchronization

### Mutex (Mutual Exclusion)
Protects shared resources from concurrent access:

```c
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

// Critical section
pthread_mutex_lock(&lock);
// Modify shared data
pthread_mutex_unlock(&lock);
```

### Race Conditions
Occur when multiple threads access shared data simultaneously:

```c
// Without mutex (UNSAFE)
global_counter++;  // Not atomic!

// With mutex (SAFE)
pthread_mutex_lock(&lock);
global_counter++;
pthread_mutex_unlock(&lock);
```

## Q1 Server Architecture

### Client State Management
```c
static int g_client_fds[MAX_CLIENTS];
static char g_client_keyword[MAX_CLIENTS][64];
static pthread_mutex_t g_lock;
```

### Workflow
1. **Accept Connection**: Server accepts client connection
2. **Create Thread**: New thread created for client
3. **Receive Keyword**: Client sends keyword + socket address
4. **Wait for All**: Thread waits for all expected clients
5. **Broadcast Result**: Server sends combined phrase to all clients

### Special Features
- **Signal Handling**: Graceful shutdown on SIGINT/SIGTERM
- **Thread Safety**: All shared data protected by mutex
- **Client Tracking**: Maintains list of connected clients
- **Keyword Normalization**: Handles case variations

## Performance Comparison

### Latency Test
```bash
# Concurrent Server (Lab 5 Q1)
# Client 1 response time: ~0.5s
# Client 2 response time: ~0.5s
# Total time: ~0.5s (parallel)

# Iterative Server (Lab 5 Q2)
# Client 1 response time: ~0.5s
# Client 2 response time: ~1.0s (waits for client 1)
# Total time: ~1.0s (sequential)
```

### Throughput Test
```bash
# Measure requests per second
time (for i in {1..100}; do ./Q2_client & done; wait)
```

## Important Functions

### Thread Management
- `pthread_create()` - Create new thread
- `pthread_join()` - Wait for thread completion
- `pthread_detach()` - Detach thread (auto cleanup)
- `pthread_mutex_lock()` - Acquire lock
- `pthread_mutex_unlock()` - Release lock

### Signal Handling
- `signal()` / `sigaction()` - Register signal handler
- Graceful cleanup on SIGINT (Ctrl+C)

## Common Issues

### 1. Deadlock
Occurs when threads wait for each other indefinitely:
```c
// Thread 1
lock(A); lock(B);  // ...

// Thread 2
lock(B); lock(A);  // DEADLOCK!
```
**Solution**: Always acquire locks in same order

### 2. Race Condition
Multiple threads modify shared data:
```c
// UNSAFE
if (counter < MAX) {
    counter++;  // Race condition!
}

// SAFE
pthread_mutex_lock(&lock);
if (counter < MAX) {
    counter++;
}
pthread_mutex_unlock(&lock);
```

### 3. Memory Leaks
Forgetting to free resources:
```c
// Ensure cleanup
pthread_mutex_destroy(&lock);
```

### 4. Port Already in Use
```bash
# Check if port is in use
netstat -tuln | grep 8080

# Kill process using port
lsof -ti:8080 | xargs kill
```

## Testing

### Test Concurrent Server
```bash
# Start server
./Q1_server 8080

# Launch multiple clients simultaneously
./Q1_client1 127.0.0.1 8080 &
./Q1_client2 127.0.0.1 8080 &
wait

# Check server output for combined phrase
```

### Test Iterative Server
```bash
# Start server
./Q2_server_iterative

# Launch clients and observe sequential handling
./Q2_client &
./Q2_client &
wait
```

### Stress Test
```bash
# Test with many clients
for i in {1..50}; do
    ./Q1_client_extra 127.0.0.1 8080 &
done
wait
```

## Debugging

### Check Thread Information
```bash
# View threads of running process
ps -eLf | grep Q1_server

# Monitor thread activity
top -H -p $(pgrep Q1_server)
```

### Memory Debugging
```bash
# Check for leaks
valgrind --leak-check=full ./Q1_server 8080
```

### Thread Debugging with GDB
```bash
gdb ./Q1_server
(gdb) set non-stop on
(gdb) run 8080
(gdb) info threads
(gdb) thread 2
```

## Best Practices

1. ✅ Always use mutexes for shared data
2. ✅ Minimize critical section size
3. ✅ Handle thread cleanup properly
4. ✅ Check return values from pthread functions
5. ✅ Use thread-safe functions (reentrant)
6. ✅ Implement timeout mechanisms
7. ✅ Graceful shutdown handling

## References

- `man 3 pthread` - POSIX threads
- `man 3 pthread_mutex` - Mutex operations
- `man 7 pthreads` - Overview of POSIX threads
- `man 2 sigaction` - Signal handling
- "Programming with POSIX Threads" by David Butenhof
