# Operating Systems — 90-Minute Class

## From Room-Sized Computers to the OS in Your Pocket

### Central idea for the class

Rather than teaching OS history as a sequence of dates, organize the class around one question:

> **What problem was the operating system invented to solve?**

Every major stage in operating-system history appeared because computers became more powerful, more expensive, more interactive, or more complicated.

A useful storyline is:

**No OS → Batch processing → Multiprogramming → Time-sharing → Unix → Personal computers → Modern OSs → Virtualization & cloud**

---

# 1. Learning objectives

By the end of the class, students should be able to:

1. Distinguish **hardware**, **software**, and an **operating system**.
2. Explain why the earliest computers did not need operating systems.
3. Describe how **batch processing**, **multiprogramming**, and **time-sharing** developed.
4. Explain how improvements in hardware enabled more sophisticated operating systems.
5. Identify the importance of **Unix** in modern OS design.
6. Explain the two fundamental roles of an operating system:

   * **resource manager**
   * **abstraction provider**
7. Identify important hardware components used by the OS:

   * CPU
   * RAM
   * storage
   * I/O devices
   * timers
   * interrupts
8. Explain how historical OS problems still appear in Linux, Windows, macOS, Android, cloud computing, and containers.

---

# 2. 90-minute lesson plan

|      Time | Topic                                           | Main idea                               |
| --------: | ----------------------------------------------- | --------------------------------------- |
|   0–8 min | Hook: Imagine a computer with no OS             | Students become the OS                  |
|  8–20 min | Hardware, software and OS fundamentals          | What exactly does an OS manage?         |
| 20–32 min | 1940s–1950s: Computers before operating systems | One machine, one program                |
| 32–48 min | Batch systems → multiprogramming → time-sharing | Improving utilization and interactivity |
| 48–60 min | Unix and the personal computer revolution       | OS ideas become standardized            |
| 60–70 min | Modern operating systems                        | Linux, Windows, macOS, mobile           |
| 70–80 min | Linux demonstration + small C example           | See historical ideas in a modern OS     |
| 80–87 min | Misconceptions + classroom discussion           | Challenge students' assumptions         |
| 87–90 min | Summary                                         | The evolution in one mental model       |

**Total: 90 minutes**

---

# 3. Opening hook — 0–8 min

Start without defining "operating system."

Tell the students:

> Imagine the university buys a computer for several million dollars.
>
> There is no Linux.
> No Windows.
> No terminal.
> No files.
> No processes.
> No mouse.
> No `printf()`.
>
> You want to execute a program.
>
> What do you do?

Give them a moment to think.

Then explain that early programmers were much closer to this situation than modern programmers.

A programmer might need to:

1. reserve time on the computer;
2. physically load a program;
3. configure the machine;
4. run the program;
5. collect the output;
6. remove the program;
7. allow the next programmer to use the machine.

Ask:

> If twenty students need to use this computer, who decides who goes next?

Someone has to coordinate everything.

That "someone" eventually became software.

**That software evolved into the operating system.**

---

# 4. Hardware, software and operating systems — 8–20 min

Before discussing history, establish three concepts.

## Hardware

Hardware is the physical computer.

Typical components:

```text
CPU
RAM
SSD / HDD
Keyboard
Mouse
Display
Network adapter
GPU
USB devices
```

### CPU

The CPU executes instructions.

A simplified view:

```text
fetch instruction
      ↓
decode instruction
      ↓
execute instruction
      ↓
fetch next instruction
```

The CPU doesn't intrinsically understand:

```text
Google Chrome
files
users
windows
processes
Docker
Spotify
```

Those are software abstractions.

---

## Memory — RAM

RAM contains the instructions and data currently being used.

For example:

```text
SSD

chrome executable
calculator executable
music.mp3
        │
        │ load
        ▼
RAM

Chrome
Calculator
OS data
        │
        ▼
CPU
```

RAM is:

* fast;
* volatile;
* directly accessible by the CPU.

---

## Storage

SSDs and HDDs provide persistent storage.

```text
RAM

fast
temporary
volatile
      ↓

SSD

slower
persistent
large
```

This distinction becomes extremely important later when studying:

* virtual memory;
* paging;
* file systems;
* caching.

---

# 5. What is software?

At a high level:

```text
Software
│
├── Application software
│   ├── Browser
│   ├── Games
│   ├── IDE
│   └── Spotify
│
└── System software
    ├── Operating system
    ├── Device drivers
    ├── compilers
    └── system utilities
```

The operating system sits between applications and hardware.

---

# Diagram 1 — The computer stack

```text
+-------------------------------------+
|           Applications              |
|                                     |
|  Chrome   GCC   Spotify   VS Code   |
+-------------------------------------+
                  │
          System calls / APIs
                  │
                  ▼
+-------------------------------------+
|          OPERATING SYSTEM           |
|                                     |
| Processes     Memory     Files       |
| Security      Network    Drivers     |
+-------------------------------------+
                  │
                  ▼
+-------------------------------------+
|             HARDWARE                |
|                                     |
| CPU   RAM   SSD   GPU   NIC   USB   |
+-------------------------------------+
```

Ask:

> Why doesn't Chrome communicate directly with the SSD whenever it wants?

Because that would make security, sharing, portability, and resource management extremely difficult.

---

# 6. Two fundamental jobs of an OS

This is one of the most important ideas of the class.

An operating system is both:

## 1. A resource manager

Hardware resources are limited.

For example:

```text
                 CPU
                  │
          ┌───────┼───────┐
          ▼       ▼       ▼
       Chrome    GCC     Spotify
```

Three programs want one CPU.

Who gets it?

The OS decides.

The same problem exists with:

* RAM;
* storage;
* network;
* GPU;
* devices.

---

## 2. An abstraction provider

Hardware is complicated.

Instead of telling a disk:

> Go to physical block 814628 and read 4096 bytes.

you say:

```c
open("grades.txt", O_RDONLY);
```

The concept of a **file** is largely an abstraction provided by the OS.

Similarly:

```text
OS abstraction      Underlying reality

process        →     CPU execution
file           →     storage blocks
virtual memory →     physical RAM
socket         →     network hardware
thread         →     scheduled execution
```

These abstractions make computers significantly easier to program.

---

# 7. Before operating systems — 20–32 min

## The 1940s

Early electronic computers were not personal computers in any recognizable sense.

There was typically:

* no operating system;
* no graphical interface;
* no terminal;
* no interactive shell;
* often no conventional programming language.

Humans performed much of what an OS does today.

A programmer effectively had control of the whole machine.

Conceptually:

```text
Programmer
     │
     ▼
+-------------+
|  Computer   |
|             |
| CPU         |
| Memory      |
| I/O         |
+-------------+
```

There was no software layer in between.

---

## The expensive-computer problem

Imagine:

```text
Computer cost: enormous
CPU usage:     extremely valuable
```

But the CPU might sit idle while an operator:

* loaded the next program;
* changed equipment;
* prepared input;
* collected output.

From the owner's perspective:

```text
$$$$$$$$ COMPUTER $$$$$$$$

CPU working:

████████

CPU waiting for humans:

████████████████████████████████
```

Not ideal.

---

# 8. Batch processing — 1950s–early 1960s

A simple solution appeared.

Don't let programmers operate the machine individually.

Instead, programmers submit **jobs**.

An operator collects them.

```text
Job 1
Job 2
Job 3
Job 4
Job 5
```

The jobs are executed sequentially.

```text
Input jobs

   ▼

+-------+
| Job A |
+-------+
    ↓
+-------+
| Job B |
+-------+
    ↓
+-------+
| Job C |
+-------+
```

This is **batch processing**.

The computer can automatically move from one job to another.

---

## Why was this important?

It reduced wasted machine time.

But imagine discovering a bug.

You submit:

```c
printf("Hello world\n");
```

Hours later you receive:

```text
Compilation failed.
```

You fix it.

Submit again.

Wait again.

Students usually appreciate their compiler at this point.

---

# 9. The CPU is still bored

Batch systems solved one problem, but another appeared.

Consider a program reading from a slow I/O device.

```text
CPU

RUN PROGRAM
████████

WAIT FOR I/O
..................

RUN PROGRAM
████████
```

While the program waits for I/O:

> **The expensive CPU is doing nothing.**

Engineers asked:

> Why can't the CPU execute another program while this program waits?

Excellent question.

---

# 10. Multiprogramming — 32–40 min

Enter **multiprogramming**.

Keep several programs in memory simultaneously.

```text
RAM

+----------------------+
| Operating System     |
+----------------------+
| Program A            |
+----------------------+
| Program B            |
+----------------------+
| Program C            |
+----------------------+
| Free memory          |
+----------------------+
```

Suppose:

```text
Program A → waiting for disk

Program B → ready to execute

Program C → waiting for input
```

The OS runs B.

This dramatically improves CPU utilization.

---

## But multiprogramming creates new problems

Suddenly the OS must answer:

```text
Which program gets the CPU?

Who gets which memory?

Can Program A access Program B's memory?

What if one program never stops?

What happens when two programs use the disk?
```

Sound familiar?

These become entire OS topics:

```text
CPU scheduling
memory management
protection
process isolation
synchronization
I/O management
```

The complexity of the OS grows because computers are doing more things simultaneously.

---

# 11. A crucial hardware invention: interrupts

The OS could not efficiently manage all of this without hardware support.

One critical mechanism is the **interrupt**.

Imagine the CPU is executing Program B.

The disk finishes an operation.

Instead of the CPU constantly asking:

```text
Disk finished?

Disk finished?

Disk finished?

Disk finished?
```

the hardware can effectively say:

```text
HEY CPU!

I FINISHED!
```

That's an interrupt.

Conceptually:

```text
Program executing
      │
      │
      ▼
+-------------+
|    CPU      |
+-------------+
      ▲
      │ INTERRUPT
      │
+-------------+
| Disk / NIC  |
+-------------+
```

The CPU temporarily transfers control to the OS.

---

# 12. Time-sharing — 40–48 min

Multiprogramming improved efficiency.

But users still wanted something else:

> interaction.

Suppose ten students connect to one computer.

The OS gives each user a small amount of CPU time.

```text
Time →

User A ███
User B    ███
User C       ███
User A          ███
User B             ███
```

Each user feels like they have their own computer.

This is **time-sharing**.

---

## Classroom analogy

Ask three students to pretend to be programs.

You are the CPU.

Give student A the whiteboard marker for three seconds.

Then:

> STOP!

Student B gets it.

Then C.

Then A again.

Congratulations.

You just implemented an extremely questionable CPU scheduler.

But the idea is correct.

---

# Diagram 2 — OS evolution

```text
1940s
NO OPERATING SYSTEM
│
│ Humans control hardware directly
▼
1950s
BATCH SYSTEMS
│
│ Automatically execute jobs
▼
1960s
MULTIPROGRAMMING
│
│ Several programs in memory
▼
1960s
TIME SHARING
│
│ Interactive users
▼
1970s
UNIX
│
│ Processes, pipes, files, portability
▼
1980s
PERSONAL COMPUTERS
│
│ One computer per user
▼
1990s
NETWORKED OPERATING SYSTEMS
│
│ Windows / Linux / Internet
▼
2000s
MOBILE + VIRTUALIZATION
│
▼
2010s–2020s
CLOUD + CONTAINERS
```

An important point:

**Old ideas didn't disappear.**

They became layers underneath newer systems.

---

# 13. Unix — 48–55 min

One of the most influential developments in OS history was Unix.

Unix developed an approach built around relatively small, composable concepts.

Some ideas strongly associated with the Unix tradition include:

```text
processes
hierarchical filesystems
pipes
shells
permissions
small composable tools
```

Students will encounter the influence of Unix everywhere in this course.

For example:

```bash
ls | grep ".c" | wc -l
```

This tiny command demonstrates one of the most powerful Unix ideas:

> Programs can be composed.

```text
ls
 │
 ▼
grep
 │
 ▼
wc
```

Output from one program becomes input to another.

---

## The famous Unix philosophy

A simplified version:

> Make programs do one thing well and allow them to work together.

Examples:

```bash
cat
grep
sort
uniq
wc
find
```

Instead of one enormous program doing everything, small programs can be combined.

---

# 14. Another historically important Unix idea: portability

Early operating systems were often closely tied to specific hardware.

A major milestone was writing much of Unix in **C**.

Conceptually:

```text
Machine A

Unix
 ↓
Hardware A
```

versus:

```text
             C
             │
       ┌─────┴─────┐
       ▼           ▼

Machine A        Machine B
Unix             Unix
Hardware A       Hardware B
```

This relationship between **C and Unix** is one reason C remains central to operating-systems education.

---

# 15. The personal computer revolution — 55–60 min

Then something changed dramatically.

Computers became cheap enough that the problem changed from:

> How can many people efficiently share one computer?

to:

> What should one person do with their own computer?

Early personal-computer operating systems could therefore be simpler.

Eventually users demanded:

* graphical interfaces;
* multitasking;
* networking;
* security;
* multimedia;
* USB;
* Wi-Fi.

Modern desktop operating systems again became enormously sophisticated.

Examples include:

```text
Windows
macOS
Linux
```

---

# 16. Modern operating systems — 60–70 min

Today a normal laptop might simultaneously run:

```text
Browser
Discord
Spotify
VS Code
Docker
Zoom
Operating system services
```

Yet perhaps the machine only has:

```text
8 CPU cores
```

while:

```text
hundreds of processes
thousands of threads
```

exist.

The illusion created by the OS is remarkable.

Each process roughly feels like:

```text
"I have a CPU."

"I have memory."

"I have files."

"I have a network."
```

Reality:

```text
                   Physical CPU
                       │
         ┌─────────────┼─────────────┐
         ▼             ▼             ▼
      Process A     Process B     Process C


                   Physical RAM
                       │
         ┌─────────────┼─────────────┐
         ▼             ▼             ▼
      Process A     Process B     Process C
```

The OS creates useful **virtual abstractions** over limited physical resources.

---

# 17. Historical connection to virtualization and cloud

A student might think:

> Virtual machines and cloud computing are completely new ideas.

Many underlying concepts are much older.

A modern server might look like:

```text
Applications
────────────────────────
Containers
────────────────────────
Linux
────────────────────────
Virtual Machine
────────────────────────
Hypervisor
────────────────────────
Physical server
```

The same historical goal keeps appearing:

> **Share expensive hardware efficiently while isolating users and programs.**

The scale changed.

The fundamental problem did not.

---

# 18. Linux demonstration — 70–80 min

Now connect the history to the computer in front of the students.

## See the hardware

```bash
lscpu
```

Discuss:

```text
architecture
cores
threads
CPU model
virtualization support
```

---

## See memory

```bash
free -h
```

Example:

```text
               total   used   free
Mem:            16Gi   6Gi    4Gi
```

Ask:

> Why doesn't each process simply receive physical RAM addresses?

Excellent preview for virtual memory.

---

## See storage

```bash
lsblk
```

Possible output:

```text
nvme0n1
├─nvme0n1p1
├─nvme0n1p2
└─nvme0n1p3
```

---

## See processes

```bash
ps aux
```

or:

```bash
ps -ef
```

Ask:

> Do we have more processes than CPU cores?

Almost certainly.

Then ask:

> So how are all these processes running?

This reconnects directly to multiprogramming and scheduling.

---

## See the kernel

```bash
uname -a
```

and:

```bash
uname -r
```

For Linux systems:

```bash
cat /proc/version
```

---

# 19. Tiny C demonstration

Use a small program to show that even a trivial application depends on OS services.

```c
#include <stdio.h>
#include <unistd.h>

int main(void)
{
    pid_t pid = getpid();

    printf("Hello from process %ld\n", (long)pid);

    return 0;
}
```

Compile:

```bash
gcc process.c -Wall -Wextra -o process
```

Run:

```bash
./process
```

Possible result:

```text
Hello from process 18432
```

Run it again:

```text
Hello from process 18436
```

Ask:

> Who created that process ID?

The operating system.

Now:

```bash
strace ./process
```

Students will see that a tiny program causes interactions with the OS.

Depending on the Linux distribution and C library, exact `strace` output will differ.

---

# 20. Why this history matters

Students sometimes ask:

> Why learn ancient operating systems when we're using Linux in 2026?

Because modern OS concepts make much more sense when you understand the problems that created them.

For example:

| Historical problem                       | Modern OS concept      |
| ---------------------------------------- | ---------------------- |
| CPU sitting idle                         | Multiprogramming       |
| Multiple programs want CPU               | Scheduling             |
| Programs must coexist                    | Processes              |
| Programs must not corrupt each other     | Memory protection      |
| User programs shouldn't control hardware | Kernel mode            |
| Slow devices                             | Interrupts             |
| Many users                               | Permissions / security |
| Programs need communication              | IPC                    |
| Hardware is complicated                  | Device drivers         |
| Storage is complicated                   | Filesystems            |
| Hardware is expensive                    | Virtualization / cloud |

Operating systems are essentially a **history of engineering solutions to resource-sharing problems.**

---

# 21. Important terminology

Students should leave class comfortable with these terms.

### Hardware

Physical components of a computer.

Examples:

```text
CPU
RAM
SSD
network card
GPU
```

### Software

Instructions executed by hardware.

---

### Operating system

Software that manages hardware resources and provides abstractions/services to programs.

---

### Kernel

The privileged core of the operating system.

It handles things such as:

```text
process scheduling
memory management
device access
system calls
interrupts
```

---

### Process

A program in execution.

---

### Program

Executable code stored somewhere such as a filesystem.

Important distinction:

```text
Program

gcc
chrome
spotify

        ↓ execute

Process

running instance of gcc
running Chrome
running Spotify
```

---

### Batch processing

Executing a collection of jobs with little or no interactive user involvement.

---

### Multiprogramming

Keeping multiple programs available so the CPU can execute another when one cannot proceed.

---

### Time-sharing

Rapidly sharing CPU execution among users or processes to provide interactive computing.

---

### Interrupt

A mechanism that allows hardware or events to request CPU attention.

---

### System call

A controlled mechanism through which a user program requests a service from the kernel.

Examples include operations related to:

```text
open
read
write
fork
exec
```

---

### Device driver

Software that enables the OS to communicate with specific hardware.

---

### Virtualization

Providing a virtual representation of computing resources.

---

# 22. Common misconceptions

## Misconception 1: "The operating system is the GUI."

No.

```text
GNOME
Windows Explorer
macOS Finder
```

are visible interfaces.

The OS also contains much deeper mechanisms such as:

```text
scheduler
virtual memory
filesystems
network stack
device drivers
```

---

## Misconception 2: "Linux is the same thing as Ubuntu."

More precisely:

```text
Linux = kernel

Ubuntu = Linux distribution
```

Ubuntu combines Linux with many other pieces of software.

---

## Misconception 3: "A program and process are the same thing."

No.

A program is executable code.

A process is a running instance.

You can execute:

```bash
./program
./program
./program
```

and create three processes from one program.

---

## Misconception 4: "If I have 8 CPU cores, only 8 programs can run."

Many more programs can appear to run concurrently because the scheduler shares CPU time.

---

## Misconception 5: "Operating systems were created primarily to make computers easier to use."

Ease of use matters, but historically **efficient resource utilization** was an enormous motivation.

Early computers were extremely expensive resources.

---

## Misconception 6: "Modern concepts replaced old OS concepts."

Usually they built upon them.

Cloud machines still have:

```text
processes
schedulers
memory
files
interrupts
device drivers
```

The abstractions are simply layered.

---

# 23. Classroom questions

### Question 1

Suppose a computer can only execute one program at a time.

The program requests data from a disk and must wait 100 ms.

What should the CPU do?

Expected direction:

> Execute another ready program.

This motivates **multiprogramming**.

---

### Question 2

Why shouldn't every application be allowed to directly control the physical disk?

Possible answers:

* corruption;
* security;
* conflicts;
* portability;
* simultaneous access;
* complexity.

This motivates **OS abstraction and protection**.

---

### Question 3

A laptop has 8 CPU cores but:

```text
350 processes
```

according to `ps`.

How is that possible?

Expected discussion:

* not every process executes simultaneously;
* many are sleeping;
* scheduler shares processors;
* processes frequently wait for I/O;
* multiple CPU cores allow real parallel execution for some tasks.

---

# 24. Programming exercise 1 — Observe multiprogramming

Have students create:

```c
#include <stdio.h>
#include <unistd.h>

int main(void)
{
    for (int i = 0; i < 20; i++) {
        printf("PID %ld: iteration %d\n",
               (long)getpid(), i);

        usleep(100000);
    }

    return 0;
}
```

Compile:

```bash
gcc worker.c -Wall -Wextra -o worker
```

Then run several simultaneously:

```bash
./worker &
./worker &
./worker &
```

Ask students to explain why the output becomes interleaved.

Concept:

**Multiple processes competing for execution.**

Exact ordering is nondeterministic and depends on scheduling.

---

# 25. Programming exercise 2 — Explore hardware versus OS abstractions

Students should collect:

```bash
lscpu
free -h
lsblk
ps -e
```

Then answer:

1. How many logical CPUs does your machine have?
2. How many processes are currently present?
3. Are there more processes than CPUs?
4. How much physical RAM exists?
5. What storage devices does the OS expose?
6. Which information represents **physical hardware**, and which represents **OS abstractions**?

Goal:

Connect hardware resources to the abstractions they will study later.

---

# 26. Optional entertaining activity: "Design an OS in 5 minutes"

Give the students this scenario:

> It is 1965.
>
> Your computer costs millions of dollars.
>
> Ten researchers want to use it.
>
> It has one CPU.
>
> Programs occasionally wait for a slow disk.
>
> One badly written program sometimes loops forever.

Ask teams:

> What features would you add?

Students may propose:

```text
Queue programs
        ↓
Batch processing

Run something else during I/O
        ↓
Multiprogramming

Interrupt long-running programs
        ↓
Timer interrupts / scheduling

Prevent programs reading other memory
        ↓
Memory protection

Give each researcher CPU time
        ↓
Time-sharing
```

Then reveal:

> Congratulations. You just rediscovered about twenty years of operating-system research.

This makes an excellent transition into the rest of the course.

---

# 27. Homework assignment

## "Why does this feature exist?"

Students choose **one modern OS feature**:

```text
processes
virtual memory
filesystems
permissions
multitasking
device drivers
containers
virtual machines
system calls
```

They must write approximately 1–2 pages answering:

1. What problem does the feature solve?
2. What would computing be like without it?
3. What historical development made it necessary?
4. What hardware support does it depend on?
5. Where can the feature be observed on Linux?

They should include at least one Linux command demonstrating it.

For example:

```bash
ps
ls -l
free
lsblk
lscpu
uname
```

---

# 28. Final three-minute summary

End with this diagram:

```text
                COMPUTER SYSTEM

+---------------------------------------+
|              Applications             |
+---------------------------------------+
                    │
                    │ abstractions
                    ▼
+---------------------------------------+
|           OPERATING SYSTEM            |
|                                       |
|  Processes   Memory   Files   Network |
|  Security    Drivers  Scheduling      |
+---------------------------------------+
                    │
                    │ manages
                    ▼
+---------------------------------------+
|               Hardware                |
|                                       |
|     CPU     RAM     SSD     Devices   |
+---------------------------------------+
```

Then give students the historical sequence one final time:

```text
No OS
  ↓
Humans manually control computers
  ↓
Batch processing
  ↓
Automate jobs
  ↓
Multiprogramming
  ↓
Keep the CPU busy
  ↓
Time-sharing
  ↓
Make computers interactive
  ↓
Unix
  ↓
Powerful abstractions + portability
  ↓
Personal computers
  ↓
One machine per user
  ↓
Modern OS
  ↓
Multitasking + networking + security
  ↓
Virtualization / containers / cloud
```

The key takeaway should be:

> **Operating systems evolved because hardware became powerful enough to do more, while software became complicated enough that someone had to manage it.**

And that gives you a useful framing for almost the entire OS course:

```text
Processes     → How do we share the CPU?

Scheduling    → Who runs next?

Virtual memory → How do we share RAM safely?

Filesystems   → How do we organize storage?

Synchronization → How do concurrent programs cooperate?

System calls  → How do programs safely ask the OS for services?

Security      → Who is allowed to do what?

Virtualization → How do we share entire computers?
```

So when students later encounter `fork()`, scheduling, page tables, locks, filesystems, and containers, they are not studying unrelated mechanisms. **They are studying successive solutions to the same historical problem: how to safely and efficiently share increasingly complex computers.**
