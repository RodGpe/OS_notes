# Operating Systems Through a Multi-Process Server

## 1. Course Overview

**Duration:** 16 weeks / approximately 4 months
**Suggested schedule:** 2 × 90-minute classes per week
**Language:** C on Linux
**Main project:** Build a production-inspired multi-process TCP/HTTP server.

### Prerequisites

Students should already understand:

* basic C programming;
* pointers;
* structs;
* arrays;
* functions;
* basic dynamic memory;
* basic Linux commands;
* compilation with `gcc`.

Networking knowledge is useful but **not required**. The networking required by the project should be introduced as part of the course.

---

# 2. Central Project

Students begin with this:

```text
Client
   │
   ▼
┌──────────┐
│  Server  │
│ process  │
└──────────┘
```

It accepts one client, processes the request, responds, and waits for the next client.

By the end of the semester they will have something closer to:

```text
                          ┌──────────────────┐
                          │      Client      │
                          └────────┬─────────┘
                                   │ TCP
                                   ▼
                        ┌─────────────────────┐
                        │   Master Process    │
                        │                     │
                        │ listen()            │
                        │ signals             │
                        │ worker supervision  │
                        └───────┬─────────────┘
                                │
             ┌──────────────────┼──────────────────┐
             │                  │                  │
             ▼                  ▼                  ▼
       ┌────────────┐     ┌────────────┐     ┌────────────┐
       │ Worker P1  │     │ Worker P2  │     │ Worker P3  │
       │            │     │            │     │            │
       │ accept()   │     │ accept()   │     │ accept()   │
       │ read()     │     │ read()     │     │ read()     │
       │ open()     │     │ open()     │     │ open()     │
       │ write()    │     │ write()    │     │ write()    │
       └──────┬─────┘     └──────┬─────┘     └──────┬─────┘
              │                  │                  │
              └────────────┬─────┴──────────────────┘
                           │
                     Shared Memory
                           │
              ┌────────────▼─────────────┐
              │ requests                 │
              │ active connections       │
              │ errors                   │
              │ bytes transferred        │
              └──────────────────────────┘

                         IPC
                          │
                          ▼
                  ┌───────────────┐
                  │ Logger Process│
                  └───────────────┘
```

Students will therefore encounter:

* system calls;
* user/kernel mode;
* processes;
* `fork()`;
* `exec()`;
* `wait()` / `waitpid()`;
* process states;
* scheduling;
* context switches;
* signals;
* zombies;
* file descriptors;
* descriptor inheritance;
* pipes;
* sockets;
* shared memory;
* semaphores;
* race conditions;
* synchronization;
* virtual memory;
* copy-on-write;
* file systems;
* blocking I/O;
* multiplexed I/O;
* resource management;
* process isolation;
* fault tolerance;
* performance measurement.

---

# 3. Final Project Specification

The final program might be executed as:

```bash
./miniserv -p 8080 -w 4 -r ./www
```

Where:

```text
-p 8080     listening port
-w 4        number of worker processes
-r ./www    directory containing files
```

Example:

```bash
curl http://localhost:8080/index.html
```

The server should support at least:

```text
GET /index.html
GET /image.jpg
GET /styles.css
```

It does **not** need to implement the entire HTTP specification.

A simplified HTTP subset is enough.

---

# 4. Course Learning Objectives

By the end of the course students should be able to:

1. Explain the difference between **user mode and kernel mode**.
2. Explain why applications require **system calls**.
3. Use Linux system calls from C.
4. Explain and use the Unix **file descriptor abstraction**.
5. Create and manage processes using `fork()`, `exec()` and `wait()`.
6. Explain process states and CPU scheduling.
7. Identify and correctly manage zombie processes.
8. Use signals for asynchronous process control.
9. Explain blocking system calls.
10. Implement IPC using pipes and shared memory.
11. Identify race conditions.
12. Use synchronization primitives.
13. Explain virtual memory and copy-on-write.
14. Explain how files are accessed through the OS.
15. Design a multi-process application.
16. Measure system performance.
17. Analyze context switching and I/O overhead.
18. Debug OS-related behavior using Linux tools.
19. Design graceful shutdown and fault recovery.
20. Relate classical OS concepts to real systems such as Nginx, Apache, PostgreSQL, Redis and container runtimes.

---

# 5. Project Evolution

The project evolves through approximately **12 versions**.

```text
V0
Hello World
    │
    ▼
V1
Sequential TCP server
    │
    ▼
V2
Sequential HTTP server
    │
    ▼
V3
fork-per-client server
    │
    ▼
V4
Signal-aware server
    │
    ▼
V5
Server + logger process
    │
    ▼
V6
Shared-memory statistics
    │
    ▼
V7
Synchronized statistics
    │
    ▼
V8
Pre-fork worker pool
    │
    ▼
V9
Static file server
    │
    ▼
V10
Robust multi-process server
    │
    ▼
V11
Performance + monitoring
    │
    ▼
V12
Final server
```

---

# MONTH 1 — Processes, Syscalls and Basic Server

---

# Week 1 — What Does an Operating System Actually Do?

## Main concepts

* Hardware vs software
* Kernel
* User space
* Kernel space
* privileged instructions
* processes
* system calls
* Linux architecture

### Central question

When C executes:

```c
write(fd, buffer, size);
```

who actually writes the data?

Not the application directly.

```text
Application
     │
     │ write()
     ▼
libc
     │
     │ syscall
     ▼
──────── User / Kernel boundary ────────
     │
     ▼
Linux Kernel
     │
     ▼
Device / filesystem / socket
```

## Session 1 — 90 minutes


| Time   | Topic                       |
| ------ | --------------------------- |
| 0–15  | Why operating systems exist |
| 15–35 | User mode vs kernel mode    |
| 35–50 | Processes                   |
| 50–70 | System calls                |
| 70–80 | `strace`demonstration       |
| 80–90 | Discussion                  |

## Session 2 — Lab

Students investigate:

```bash
strace ls
```

and:

```bash
strace cat file.txt
```

Identify calls such as:

```text
openat()
read()
write()
close()
mmap()
```

### Project milestone 0

Create:

```text
miniserv/
├── Makefile
├── src/
│   └── server.c
├── include/
└── README.md
```

Compile using:

```bash
gcc -Wall -Wextra src/server.c -o miniserv
```

---

# Week 2 — File Descriptors and Blocking I/O

This week establishes one of the most important Unix ideas:

> Sockets, pipes, files and terminals can all be manipulated through file descriptors.

## Important terminology

* file descriptor
* open file description
* blocking call
* `read`
* `write`
* `open`
* `close`
* socket

Conceptual view:

```text
Process

FD Table
┌────┬─────────────────┐
│ 0  │ stdin           │
│ 1  │ stdout          │
│ 2  │ stderr          │
│ 3  │ server socket   │
│ 4  │ client socket   │
└────┴─────────────────┘
```

## Networking introduction

Introduce only what the project requires:

```c
socket()
bind()
listen()
accept()
read()
write()
close()
```

### Server lifecycle

```text
socket()
   │
bind()
   │
listen()
   │
accept()
   │
read()
   │
write()
   │
close()
```

## Project milestone 1

Implement a **sequential TCP server**.

It should accept:

```bash
nc localhost 8080
```

Client:

```text
hello
```

Server responds:

```text
Hello from MiniServ
```

### Important observation

While the server executes:

```c
read(client_fd, buffer, sizeof(buffer));
```

the process may become:

```text
RUNNING
   │
read()
   ▼
BLOCKED
   │
data arrives
   ▼
READY
   │
scheduled
   ▼
RUNNING
```

This is the first direct connection between the project and **process scheduling**.

---

# Week 3 — Process Creation

## Concepts

* `fork()`
* parent
* child
* PID
* PPID
* process tree
* address-space duplication
* `wait()`
* `waitpid()`

Example:

```c
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void)
{
    pid_t pid = fork();

    if (pid == 0) {
        printf("Child: PID=%d\n", getpid());
        return 0;
    }

    printf("Parent: PID=%d child=%d\n",
           getpid(), pid);

    wait(NULL);

    return 0;
}
```

Compile:

```bash
gcc -Wall fork_demo.c -o fork_demo
```

Run:

```bash
./fork_demo
```

Possible output:

```text
Parent: PID=4000 child=4001
Child: PID=4001
```

The order may vary because the scheduler decides which process runs first.

## Project milestone 2

Replace:

```text
one process
```

with:

```text
               fork()
                 │
                 ▼
Client ───► Parent
             │
             ├── Child handles Client A
             │
             ├── Child handles Client B
             │
             └── Child handles Client C
```

Pseudo-code:

```c
while (1) {

    client = accept(...);

    pid = fork();

    if (pid == 0) {

        close(server_fd);

        handle_client(client);

        close(client);

        exit(0);
    }

    close(client);
}
```

This is the project's first **real multi-process architecture**.

---

# Week 4 — Descriptor Inheritance and Zombie Processes

Students discover a subtle but fundamental consequence of `fork()`.

After:

```c
client = accept(...);
fork();
```

both processes inherit descriptors.

```text
Before fork()

Parent
FD 3 → server socket
FD 4 → client

        fork()
          │
          ▼

Parent                 Child
FD3 → server socket    FD3 → server socket
FD4 → client           FD4 → client
```

Therefore proper cleanup matters.

Parent:

```c
close(client_fd);
```

Child:

```c
close(server_fd);
```

## Zombies

Demonstrate:

```text
Parent
  │
  ├── Child exits
  │
  │
  └── ZOMBIE
  │
wait()
  │
  ▼
removed
```

Observe with:

```bash
ps aux
```

or:

```bash
ps -o pid,ppid,state,cmd
```

## Project requirement

Students intentionally create a zombie bug.

They must:

1. reproduce it;
2. observe it;
3. explain it;
4. fix it.

---

# MONTH 2 — Signals, IPC and Concurrency

# Week 5 — Signals

## Concepts

* asynchronous events
* `SIGINT`
* `SIGTERM`
* `SIGCHLD`
* `SIGPIPE`
* `sigaction()`
* interrupted system calls
* `EINTR`

Server problem:

```bash
Ctrl+C
```

Should the master simply disappear?

No.

Students implement graceful shutdown.

```text
SIGTERM
   │
   ▼
Master
   │
   ├── tell workers to stop
   │
   ├── wait for workers
   │
   ├── close listening socket
   │
   └── exit
```

Use `sigaction()` rather than teaching `signal()` as the primary API.

## Project milestone 3

Implement:

* `SIGCHLD` handling;
* `SIGINT`;
* `SIGTERM`;
* child cleanup;
* graceful server termination.

---

# Week 6 — Scheduling and Context Switching

Now students have enough processes to make scheduling meaningful.

## Topics

* CPU scheduler
* ready queue
* running
* waiting
* time slice
* preemption
* context switch
* CPU-bound process
* I/O-bound process

Example:

```text
Ready Queue

P2 → P5 → P8 → P3
          │
          ▼
        CPU
       ┌────┐
       │ P1 │
       └────┘
```

Discuss why the server is generally:

```text
I/O bound
```

rather than:

```text
CPU bound
```

## Experiment

Compare:

```text
Sequential server
```

against:

```text
fork-per-client
```

with multiple clients.

Students can generate concurrent requests with:

```bash
seq 1 20 | xargs -n1 -P20 -I{} curl -s http://127.0.0.1:8080/
```

They measure:

```bash
time ...
```

They should explain why concurrency improves some workloads but also introduces:

* scheduling overhead;
* memory overhead;
* context switching.

---

# Week 7 — Interprocess Communication: Pipes

Workers now need to communicate.

## IPC concepts

Introduce:

* anonymous pipes;
* named pipes;
* Unix sockets;
* shared memory;
* signals.

Start with pipes.

```text
Worker
   │
   │ pipe
   ▼
Logger
```

Example architecture:

```text
Worker 1 ───┐
Worker 2 ───┼──► Logger Process ───► server.log
Worker 3 ───┘
```

A worker generates:

```text
GET /index.html 200 1024
```

Instead of writing directly to the log file, it sends a message to a dedicated logger.

## Project milestone 4

Create:

```text
logger.c
logger.h
```

Workers send log messages through IPC.

Students must explain why this architecture can reduce concurrent modifications of the log file.

---

# Week 8 — Race Conditions and Synchronization

This is one of the most important weeks.

Suppose all workers increment:

```c
requests++;
```

Students may assume this is one operation.

It isn't conceptually atomic.

```text
requests++

LOAD requests
ADD 1
STORE requests
```

Two workers:

```text
Worker A                 Worker B

LOAD 10
                         LOAD 10
ADD 1
                         ADD 1
STORE 11
                         STORE 11
```

Expected:

```text
12
```

Actual:

```text
11
```

## Topics

* race condition
* critical section
* atomicity
* mutual exclusion
* semaphore
* mutex
* deadlock

Because processes do not naturally share normal global variables, introduce POSIX shared memory or anonymous shared mappings.

---

# Week 9 — Shared Memory

## Concept

Normally:

```text
Process A memory       Process B memory

counter = 10           counter = 10

      separate address spaces
```

With shared memory:

```text
Process A
     │
     ▼
┌───────────────┐
│ Shared memory │
│ counter = 10  │
└───────────────┘
     ▲
     │
Process B
```

Example using:

```c
mmap(
    NULL,
    sizeof(struct stats),
    PROT_READ | PROT_WRITE,
    MAP_SHARED | MAP_ANONYMOUS,
    -1,
    0
);
```

## Final statistics structure

For example:

```c
struct server_stats {
    unsigned long requests;
    unsigned long errors;
    unsigned long bytes_sent;
    unsigned int active_clients;
};
```

## Project milestone 5

Workers update shared statistics.

Initially implement it **without synchronization**.

Students run a stress test.

Then add synchronization.

This provides a concrete demonstration of a race condition instead of a purely theoretical one.

---

# MONTH 3 — Memory, Process Pools, Filesystems and I/O

# Week 10 — Virtual Memory and Copy-on-Write

Now revisit `fork()` more deeply.

Students initially imagine:

```text
fork()

copy EVERYTHING
```

Modern systems instead heavily rely on **copy-on-write**.

Initially:

```text
Parent Virtual Memory
        │
        ▼
      Page A
        ▲
        │
Child Virtual Memory
```

If the child modifies the page:

```text
Parent          Child

 Page A        Page B
   ▲             ▲
   │             │
private        private
```

## Topics

* virtual address
* physical address
* pages
* page tables
* page fault
* copy-on-write
* demand paging
* RSS
* VSZ

Inspect the server:

```bash
cat /proc/<PID>/maps
```

and:

```bash
cat /proc/<PID>/status
```

Use:

```bash
ps -o pid,ppid,vsz,rss,stat,cmd
```

## Connection to server architecture

Why might:

```text
fork 10,000 processes
```

be problematic even with copy-on-write?

Students analyze resource usage.

---

# Week 11 — Pre-Fork Worker Pool

The existing architecture has a major problem:

```text
request
   │
 fork()
   │
 child
   │
request finished
   │
 child exits
```

Creating processes continuously has overhead.

Introduce:

```text
             Master

               │
      fork at startup
               │
     ┌─────────┼─────────┐
     │         │         │
 Worker 1   Worker 2   Worker 3
     │         │         │
 accept()   accept()   accept()
```

The workers already exist.

## Comparison

### Fork per request

```text
accept
fork
serve
exit
```

### Pre-fork

```text
startup

fork
fork
fork
fork

then workers repeatedly:

accept
serve
accept
serve
```

## Project milestone 6

Implement:

```bash
./miniserv -w 4
```

which creates exactly four worker processes.

Students confirm:

```bash
ps -ef | grep miniserv
```

Example:

```text
5010 miniserv master
5011 miniserv worker
5012 miniserv worker
5013 miniserv worker
5014 miniserv worker
```

This introduces a real architectural idea used historically by high-performance servers.

---

# Week 12 — Filesystems

The server becomes a real static web server.

Request:

```http
GET /hello.txt HTTP/1.1
```

Becomes:

```text
HTTP Request
     │
     ▼
parse path
     │
     ▼
open()
     │
     ▼
filesystem
     │
     ▼
read()
     │
     ▼
write(socket)
```

## Topics

* filesystem abstraction;
* file descriptors;
* inode;
* metadata;
* directories;
* file offsets;
* permissions;
* page cache;
* `stat()`.

Example:

```c
int fd = open("www/index.html", O_RDONLY);

read(fd, buffer, sizeof(buffer));

write(client_fd, buffer, bytes);

close(fd);
```

Students must serve both:

```text
text
images
CSS
binary files
```

They learn why code like:

```c
strlen(buffer)
```

cannot safely determine the length of arbitrary binary data.

## Project milestone 7

Implement static files:

```text
www/
├── index.html
├── image.jpg
├── style.css
└── hello.txt
```

---

# Week 13 — Blocking I/O and Multiplexing

Return to:

```c
read()
```

Suppose a malicious or slow client connects and sends nothing.

```text
Worker 1
   │
accept()
   │
read()
   │
BLOCKED
   │
   │ client waits for 10 minutes
   ▼
worker unavailable
```

Introduce:

* blocking;
* nonblocking mode;
* `select`;
* `poll`;
* `epoll`;
* readiness notification.

Students don't necessarily need to build the complete server around `epoll`, but they should understand the architectural difference.

### Process-per-client

```text
Client A ── Worker A
Client B ── Worker B
Client C ── Worker C
```

### Multiplexed

```text
Client A ─┐
Client B ─┼──► epoll ─► Worker
Client C ─┘
```

## Optional advanced milestone

Each worker handles multiple clients using:

```c
epoll()
```

This can be worth bonus credit.

---

# MONTH 4 — Reliability, Security and Performance

# Week 14 — Resource Management and Server Robustness

Students try to break their own server.

Topics:

* file descriptor exhaustion;
* process limits;
* memory limits;
* `RLIMIT_NOFILE`;
* `RLIMIT_NPROC`;
* `SIGPIPE`;
* failed system calls;
* partial reads;
* partial writes;
* error handling.

Inspect limits:

```bash
ulimit -a
```

File descriptors:

```bash
ls /proc/<pid>/fd
```

Open sockets:

```bash
ss -ltnp
```

## Security topics

Discuss:

```text
GET /../../../../etc/passwd
```

Why this request must never allow arbitrary filesystem access.

Introduce:

* least privilege;
* path validation;
* privilege separation;
* resource limits;
* process isolation.

## Project milestone 8

Server must survive:

* invalid requests;
* client disconnects;
* missing files;
* large requests;
* rapid connections;
* worker crashes.

---

# Week 15 — Process Supervision and Fault Tolerance

Now intentionally crash a worker:

```c
abort();
```

What should happen?

Bad architecture:

```text
Worker dies
    │
    ▼
server permanently loses capacity
```

Better architecture:

```text
Worker dies
    │
    ▼
SIGCHLD
    │
    ▼
Master detects death
    │
    ▼
fork()
    │
    ▼
new worker
```

Final supervisor:

```text
                Master
                  │
          ┌───────┼───────┐
          ▼       ▼       ▼
          W1      W2      W3
                  │
                 CRASH
                  │
                  ▼
               SIGCHLD
                  │
                  ▼
                Master
                  │
                fork()
                  │
                  ▼
                 W4
```

## Project milestone 9

The master process must maintain the configured number of workers.

Example:

```bash
kill -9 <worker_pid>
```

Then:

```bash
ps
```

should show that a replacement worker appeared.

---

# Week 16 — Performance, Final Integration and Presentations

Students compare architectures.

## Experiment 1

Sequential:

```text
1 process
```

## Experiment 2

Fork per connection:

```text
1 child per request
```

## Experiment 3

Pre-fork:

```text
4 persistent workers
```

Optional:

```text
4 workers + epoll
```

Students analyze:

* throughput;
* latency;
* CPU usage;
* memory;
* process count;
* context switches.

Useful Linux tools:

```bash
time
ps
top
strace
lsof
ss
/proc
```

They should not merely report:

```text
Server B is faster.
```

They should explain **why**.

---

# 6. Final Project Architecture

A strong final implementation could look like:

```text
                         INTERNET
                            │
                            ▼
                     Listening Socket
                            │
                            ▼
                    ┌──────────────┐
                    │    MASTER    │
                    │              │
                    │ supervision  │
                    │ signals      │
                    │ workers      │
                    └──────┬───────┘
                           │
             ┌─────────────┼─────────────┐
             │             │             │
             ▼             ▼             ▼
          Worker 1      Worker 2      Worker 3
             │             │             │
             └──────┬──────┴──────┬──────┘
                    │             │
                    ▼             ▼
              Shared Memory      Pipe
                    │             │
                    ▼             ▼
               Statistics      Logger
                                   │
                                   ▼
                              access.log

Workers
   │
   ├── accept()
   ├── read()
   ├── parse request
   ├── open()
   ├── read file
   ├── write response
   ├── update shared statistics
   └── send log message
```

---

# 7. Suggested Source-Code Architecture

By the final month:

```text
miniserv/
│
├── Makefile
│
├── README.md
│
├── include/
│   ├── server.h
│   ├── worker.h
│   ├── http.h
│   ├── logger.h
│   ├── stats.h
│   └── signals.h
│
├── src/
│   ├── main.c
│   ├── server.c
│   ├── worker.c
│   ├── http.c
│   ├── logger.c
│   ├── stats.c
│   └── signals.c
│
├── www/
│   ├── index.html
│   ├── style.css
│   └── image.jpg
│
└── tests/
    ├── basic.sh
    ├── concurrency.sh
    └── stress.sh
```

---

# 8. Project Milestones


| Milestone | Week | Deliverable                 |
| --------- | ---: | --------------------------- |
| M0        |    1 | Project skeleton            |
| M1        |    2 | Sequential TCP server       |
| M2        |    3 | Fork-per-client             |
| M3        |    5 | Signals + zombie management |
| M4        |    7 | Logger IPC                  |
| M5        |    9 | Shared statistics           |
| M6        |   11 | Pre-fork process pool       |
| M7        |   12 | Static HTTP files           |
| M8        |   14 | Robustness/security         |
| M9        |   15 | Worker supervision          |
| Final     |   16 | Complete server             |

I recommend **grading each milestone individually** rather than grading only the final result.

That prevents students from trying to build the complete server during the final week.

---

# 9. Suggested Grading


| Component                      | Weight |
| ------------------------------ | -----: |
| Labs                           |    20% |
| Quizzes / conceptual questions |    10% |
| Project milestones             |    35% |
| Final server                   |    20% |
| Final technical report         |    10% |
| Presentation / oral defense    |     5% |

---

# 10. Final Project Rubric

## Process management — 20%

Students correctly implement:

* `fork()`;
* workers;
* `waitpid()`;
* zombie prevention;
* worker restart.

## File descriptor management — 15%

Correct use of:

* socket descriptors;
* client descriptors;
* files;
* pipes;
* `close()`.

No obvious descriptor leaks.

## IPC — 15%

Use of:

* pipe/socketpair;
* shared memory;
* process communication.

## Synchronization — 10%

Correct protection of shared state.

Students must explain:

> What race condition would exist without the synchronization?

## Signals — 10%

Correct handling of:

* `SIGTERM`;
* `SIGINT`;
* `SIGCHLD`;
* optionally `SIGHUP`.

## Filesystem / HTTP — 10%

Correctly serves static files.

## Reliability — 10%

Handles:

* invalid requests;
* disconnects;
* worker death;
* missing files.

## Analysis — 10%

Performance report comparing architectures.

---

# 11. Required Experiments

Students should include experimental evidence in their report.

## Experiment A — Sequential vs Concurrent

Compare:

```text
Sequential

vs

fork-per-client

vs

pre-fork
```

---

## Experiment B — Worker count

Test:

```text
1 worker
2 workers
4 workers
8 workers
16 workers
```

Ask:

> Does doubling the number of workers always double performance?

It should lead directly into scheduling and resource contention.

---

## Experiment C — Memory

Observe:

```bash
ps -o pid,ppid,rss,vsz,stat,cmd
```

Compare worker memory.

Discuss:

* virtual memory;
* physical memory;
* copy-on-write.

---

## Experiment D — System calls

Run:

```bash
strace -f ./miniserv -p 8080 -w 4 -r ./www
```

Students identify:

```text
socket
bind
listen
accept
clone/fork
read
write
openat
close
wait
```

---

# 12. Important Terminology Students Should Know

By the end of the course:

```text
Kernel
User mode
Kernel mode
System call
Trap
Interrupt
Process
PID
PPID
PCB
Process state
Ready queue
Scheduler
Context switch
fork
exec
wait
Zombie
Orphan
File descriptor
Pipe
Socket
IPC
Signal
Race condition
Critical section
Semaphore
Mutex
Deadlock
Virtual memory
Page
Page table
Page fault
Copy-on-write
Shared memory
Filesystem
Inode
Blocking I/O
Nonblocking I/O
Multiplexing
Throughput
Latency
Resource limit
```

---

# 13. Common Misconceptions to Explicitly Address

### "`fork()` creates a thread."

No.

It creates another **process**.

---

### "`fork()` starts another program."

Not necessarily.

The new process initially continues executing the same program.

`exec()` replaces the program.

---

### "`write()` writes directly to the hardware."

Usually no.

It crosses into the kernel, which manages the underlying resource.

---

### "One TCP client equals one socket."

On the server there is generally:

```text
listening socket
```

plus:

```text
connected socket
```

for each accepted connection.

---

### "If a process exits, it automatically disappears."

Not necessarily.

Without the parent's `wait()`, it can temporarily remain as a zombie.

---

### "`counter++` is atomic."

Do not assume that.

---

### "Processes cannot share memory."

Their normal address spaces are isolated, but the OS can explicitly map shared memory between them.

---

### "More processes always means better performance."

Eventually additional workers create additional:

* scheduling;
* memory;
* cache;
* context switching;
* contention

costs.

---

### "Blocking is always bad."

Blocking can be extremely simple and efficient.

The problem occurs when the architecture requires one blocked execution context to serve many unrelated clients.

---

# 14. Three Important Classroom Questions

### Question 1

A worker executes:

```c
read(client_fd, buffer, 1024);
```

but the client sends nothing.

What happens to the worker?

Expected discussion:

```text
RUNNING
   ↓
BLOCKED
   ↓
data arrives
   ↓
READY
   ↓
scheduler selects it
   ↓
RUNNING
```

---

### Question 2

After:

```c
int client = accept(...);

fork();
```

who owns `client`?

Answer:

Both processes initially have a descriptor referring to the underlying open socket description.

This leads into descriptor inheritance.

---

### Question 3

Why might:

```text
4 workers
```

outperform:

```text
1000 workers
```

even if 1000 clients exist?

Expected concepts:

* context switching;
* memory consumption;
* scheduler overhead;
* CPU count;
* cache behavior;
* contention;
* blocking behavior.

---

# 15. Two Short In-Class Programming Exercises

## Exercise 1 — Intentionally create zombies

Modify the server so children exit but the parent never calls `wait()`.

Generate several connections:

```bash
for i in $(seq 1 10); do
    curl http://localhost:8080/ &
done
```

Inspect:

```bash
ps -o pid,ppid,state,cmd
```

Students must identify:

```text
Z
```

and explain what it means.

Then fix the problem.

---

# Exercise 2 — Race Condition

Create shared memory containing:

```c
unsigned long counter;
```

Create four processes.

Each executes:

```c
for (int i = 0; i < 1000000; i++)
    counter++;
```

Expected theoretical result:

```text
4,000,000
```

Run it several times.

Students should observe that the result may be smaller or variable.

Then protect the critical section with a semaphore.

---

# 16. Final Homework / Final Programming Assignment

## MiniServ

Implement a Unix-style multi-process HTTP server.

The server must:

1. Create a TCP listening socket.
2. Accept HTTP connections.
3. Serve static files.
4. Use a configurable pre-fork worker pool.
5. Correctly manage file descriptors.
6. Handle `SIGINT` and `SIGTERM`.
7. Handle `SIGCHLD`.
8. Automatically replace dead workers.
9. Maintain server statistics in shared memory.
10. Synchronize shared statistics.
11. Send logs to another process using IPC.
12. Shut down gracefully.
13. Avoid zombie processes.
14. Reject invalid file paths.
15. Survive disconnected clients.
16. Return reasonable HTTP errors such as:

```text
400 Bad Request
404 Not Found
500 Internal Server Error
```

Students must also submit a report explaining:

```text
1. Architecture
2. Process model
3. System calls used
4. File descriptor ownership
5. IPC architecture
6. Synchronization
7. Signal handling
8. Virtual-memory behavior
9. Filesystem interaction
10. Performance experiments
11. Bugs encountered
12. Design trade-offs
```

---

# 17. Final Oral Defense

I strongly recommend an oral component.

Give the student their own code and ask questions such as:

> Why does the child close the listening descriptor here?

> What happens if this `waitpid()` is removed?

> Which process owns this pipe?

> What happens if two workers execute this line simultaneously?

> Which system call causes this process to block?

> What happens in the kernel after `accept()`?

> Why did four workers outperform one worker?

> Why didn't 32 workers outperform eight?

> What memory does the child initially share after `fork()`?

> What would happen if a worker crashed here?

This makes it difficult to submit a project the student does not understand.

---

# 18. The Most Important Architectural Progression

The real educational value is not the HTTP server itself.

It is watching the architecture change.

### Beginning of the semester

```text
Client
  │
  ▼
Server
  │
  ▼
read()
  │
  ▼
write()
```

### Processes

```text
              Server
                │
              fork
          ┌─────┼─────┐
          ▼     ▼     ▼
          P1    P2    P3
```

### IPC

```text
Workers ───► Logger
```

### Shared state

```text
Workers
   │
   ▼
Shared Memory
```

### Synchronization

```text
Workers
   │
 semaphore
   │
   ▼
Shared Memory
```

### Final system

```text
                Master
             /     |     \
           W1      W2      W3
            \      |      /
             Shared Memory
                   |
                 Stats

Workers ──IPC──► Logger

Workers ──syscalls──► Linux Kernel
                          │
           ┌──────────────┼─────────────┐
           ▼              ▼             ▼
        Network       Filesystem     Scheduler
```

At that point, students can see that a "server" is really an application built directly on top of the abstractions provided by the operating system.

---

# 19. Course Concept Map

```text
                    OPERATING SYSTEMS
                           │
       ┌───────────────────┼────────────────────┐
       │                   │                    │
    Processes            Memory               I/O
       │                   │                    │
     fork()          Virtual Memory       File Descriptors
       │                   │                    │
     wait()             Paging              Files
       │                   │                    │
    Signals             COW                Sockets
       │                   │                    │
   Scheduling        Shared Memory          Pipes
       │                   │                    │
       └───────────── Synchronization ─────────┘
                           │
                        Server
                           │
                           ▼
                 Multi-Process System
```

The project therefore becomes the **spine of the course** rather than an unrelated final assignment.

Every time students learn a new OS concept, the question becomes:

> **"What problem in our server does this operating-system mechanism solve?"**

That question can drive nearly the entire semester.
