# ASST2: System calls and processes

Table of Contents

- Due Dates and Mark Distribution
- Introduction
- User-level programs
- Design
- Existing Code Walkthrough
- Basic Assignment
  - Setup
  - Building and Testing the Provided Code
  - Configure OS/161 for Assignment 2
  - Building for ASST2
  - Command Line Arguments to OS/161
  - Running "asst2"
- The Assignment Task: File System Calls
  - Notes on the file system system calls
  - Notes on standard file descriptors
  - Some Design Questions
  - Documenting your solution
  - FAQ, Gotchas, and Video
  - Basic Assignment Submission
- Advanced Assignment
  - User-level Process Management System Calls
    - fork()
    - getpid()
    - execv(), waitpid(), _exit()
    - kill_curthread()
  - Design Questions
  - Advanced Assignment Submission

## Due Dates and Mark Distribution

Due Date & Time: 4pm, Fri Mar 31st

Marks: Worth 30 marks (of the class mark component of the course)

The 2% per day bonus for each day early applies, capped at 10%, as per course outline.

Students can do the advanced part if approved. Students obtain approval if they submit the basic assignment 5 days prior to the due date. Marks obtained are added to any shortfall in the class mark component up to a maximum of 10 bonus marks overall for all assignments.

## Introduction

In this assignment you will be implementing a software bridge between a set of file-related system calls inside the OS/161 kernel and their implementation within the VFS (obviously also inside the kernel). Upon completion, your operating system will be able to run a single application at user-level and perform some basic file I/O.

A substantial part of this assignment is understanding how OS/161 works and determining what code is required to implement the required functionality. Expect to spend at least as long browsing and digesting OS/161 code as actually writing and debugging your own code.

If you attempt the advanced part, you will add process related system calls and the ability to run multiple applications.

Your current OS/161 system has minimal support for running executables, nothing that could be considered a true process. Assignment 2 starts the transformation of OS/161 into something closer to a true operating system. After this assignment, OS/161 will be capable of running a process from actual compiled programs stored in your account. The program will be loaded into OS/161 and executed in user mode by System/161; this will occur under the control of your kernel. First, however, you must implement part of the interface between user-mode programs ("userland") and the kernel. As usual, we provide part of the code you will need. Your job is to design and build the missing pieces.

The code can run one user-level C program at a time as long as it doesn't want to do anything but shut the system down. We have provided sample user programs that do this (reboot, halt, poweroff), as well as others that make use of features you might be adding in this and future assignments. So far, all the code you have written for OS/161 has only been run within, and only been used by, the operating system kernel itself. In a real operating system, the kernel's main function is to provide support for user-level programs. Most such support is accessed via "system calls". We give you two system call implementations: sys_reboot() in main/main.c and sys___time() in syscall/time_syscalls.c. In GDB, if you put a breakpoint on sys_reboot() and run the "reboot" program, you can use "backtrace" (or "where") to see how it got there.

## User-level programs

Our System/161 simulator can run normal C programs if they are compiled with a cross-compiler, os161-gcc. A cross compiler runs on a host (e.g., a Linux x86 machine) and produces MIPS executables; it is the same compiler used to compile the OS/161 kernel. Various user-level programs already exist in userland/bin, userland/testbin, and userland/sbin. Note: that only a small subset these programs will execute successfully due to OS/161 only supporting a small subset of the system call interface.

To create new user programs (for testing purposes), you need to edit the Makefile in bin, sbin, or testbin (depending on where you put your programs) and then create a directory similar to those that already exist. Use an existing program and its Makefile as a template.

## Design

In the beginning, you should tackle this assignment by producing a DESIGN. The design should clearly reflect the development of your solution. The design should not merely be what you programmed. If you try to code first and design later, or even if you design hastily and rush into coding, you will most certainly end up in a software "tar pit". Don't do it! Plan everything you will do. Don't even think about coding until you can precisely explain to your partner what problems you need to solve and how the pieces relate to each other. Note that it can often be hard to write (or talk) about new software design, you are facing problems that you have not seen before, and therefore even finding terminology to describe your ideas can be difficult. There is no magic solution to this problem; but it gets easier with practice. The important thing is to go ahead and try. Always try to describe your ideas and designs to someone else. In order to reach an understanding, you may have to invent terminology and notation, this is fine. If you do this, by the time you have completed your design, you will find that you have the ability to efficiently discuss problems that you have never seen before. Why do you think that CS is filled with so much jargon? To help you get started, we have provided the following questions as a guide for reading through the code to comprehend what is already provided.

To get a feel for what problems you need to solve, review the design questions and design document section later in this specification.

## Existing Code Walkthrough

A guided walkthrough of the relevant code base is available here.

This walkthrough complements the existing ASST2 video. There are answers available on the course wiki.

## Basic Assignment

### Setup

We assume after ASST0 and ASST1 that you now have some familiarity with setting up for OS/161 development. If you need more detail, refer back to ASST0.

Clone the ASST2 source repository from gitlab.cse.unsw.edu.au, replacing the XXX with your 3 digit group number:

```bash
% cd ~/cs3231
% git clone https://zNNNNNNN@gitlab.cse.unsw.edu.au/COMP3231/23T1/grpXXX-asst2.git asst2-src
```

Note: The gitlab repository is shared between you and your partner. You can both push and pull changes to and from the repository to cooperate on the assignment. If you are not familiar with cooperative software development and git you should consider spending a little time familiarising yourself with git.

### Building and Testing the Provided Code

#### Configure OS/161 for Assignment 2

Before proceeding further, configure your new sources:

```bash
% cd ~/cs3231/asst2-src
% ./configure
```

Unlike the previous assignment, you will need to build and install the user-level programs that will be run by your kernel in this assignment:

```bash
% cd ~/cs3231/asst2-src
% bmake
% bmake install
```

For your kernel development, again we have provided you with a framework for you to run your solutions for ASST2. You have to reconfigure your kernel before you can use this framework. The procedure for configuring a kernel is the same as in ASST0 and ASST1, except you will use the ASST2 configuration file:

```bash
% cd ~/cs3231/asst2-src/kern/conf
% ./config ASST2
```

You should now see an ASST2 directory in the compile directory.

### Building for ASST2

When you built OS/161 for ASST1, you ran make from compile/ASST1. In ASST2, you run make from compile/ASST2:

```bash
% cd ../compile/ASST2
% bmake depend
% bmake
% bmake install
```

If you are told that the compile/ASST2 directory does not exist, make sure you ran config for ASST2.

### Command Line Arguments to OS/161

Your solutions to ASST2 will be tested by running OS/161 with command line arguments that correspond to the menu options in the OS/161 boot menu. IMPORTANT: Please DO NOT change these menu option strings.

### Running "asst2"

For this assignment, we have supplied a user-level OS/161 program that you can use for testing. It is called asst2, and its sources live in src/testbin/asst2. You can test your assignment by typing `p /testbin/asst2` at the OS/161 menu prompt. As a shortcut, you can also specify menu arguments on the command line, for example: `sys161 kernel "p /testbin/asst2"`.

If you don't have a sys161.conf file, you can use the one from ASST1. The simplest way to install it is as follows:

```bash
% cd ~/cs3231/root
% wget http://cgi.cse.unsw.edu.au/~cs3231/23T1/assignments/asst2/sys161.conf
```

Running the program produces output similar to the following prior to starting the assignment:

```
Unknown syscall 55
...
Unknown syscall 3
exit() was called, but it's unimplemented.
This is expected if your user-level program has finished.
panic: Can't continue further until sys_exit() is implemented
```

asst2 produces the following output on a (maybe partially) working assignment:

```
OS/161 kernel [? for menu]: p /testbin/asst2
Operation took 0.000212160 seconds
OS/161 kernel [? for menu]:

**********
* File Tester
**********
* write() works for stdout
**********
* write() works for stderr
**********
* opening new file "test.file"
* open() got fd 3
* writing test string
* wrote 45 bytes
* writing test string again
* wrote 45 bytes
* closing file
**********
* opening old file "test.file"
* open() got fd 3
* reading entire file into buffer
* attempting read of 500 bytes
* read 90 bytes
* attempting read of 410 bytes
* read 0 bytes
* reading complete
* file content okay
**********
* testing lseek
* reading 10 bytes of file into buffer
* attempting read of 10 bytes
* read 10 bytes
* reading complete
* file lseek  okay
* closing file
exit() was called, but it's unimplemented.
This is expected if your user-level program has finished.
panic: Can't continue further until sys_exit() is implemented
```

Note that the final panic is expected, and is due to exit() (system call 3) not being implemented completely by OS/161. Implementing exit() is part of the advanced assignment.

## The Assignment Task: File System Calls

Of the full range of system calls that is listed in `kern/include/kern/syscall.h`, your task is to implement the following file-based system calls: `open`, `read`, `write`, `lseek`, `close`, `dup2`, and document your design. You will be writing the kernel code that implements part of the system call functionality within the kernel. You are not writing the C stubs that user-level applications call to invoke the system calls. The userland stubs are automatically generated when you build OS/161 in `build/userland/lib/libc/syscalls.S` which you should not modify.

It's crucial that your syscalls handle all error conditions gracefully (without crashing OS/161) no matter what an application requests. Your code should also be memory leak free. Consult the OS/161 man pages and understand the system calls that you must implement. Your system calls must return the correct value (in case of success) or an appropriate error code (in case of failure). Some of the auto-marking scripts rely on the return of error codes; we are lenient as to the specific code in the case of potential ambiguity.

The file `userland/include/unistd.h` contains the user-level interface definition of the system calls. This interface is different from that of the kernel functions that you will define to implement these calls. You need to design the kernel side of this interface. The function prototype for your interface can be placed in `kern/include/syscall.h`. The integer codes for the calls are defined in `kern/include/kern/syscall.h`.

### Notes on the file system system calls

`open()`, `read()`, `write()`, `lseek()`, `close()`, and `dup2()`

While this assignment requires you to implement file-system-related system calls, you actually have to write virtually no low-level file system code in this assignment. You will use the existing VFS layer to do most of the work. Your job is to construct the subsystem that implements the interface expected by userland programs by invoking the appropriate VFS and vnode operations.

Although these system calls may seem to be tied to the filesystem, they are really about manipulation of file descriptors or filesystem state. A large part of this assignment is designing and implementing a system to track this state.

Some of this state is specific to a process and file descriptor (a per-process file descriptor table), and some information is shared across multiple processes (e.g., an open file table). Do not rush this design. Think carefully about the state you need to maintain, how to organise it, and when and how it has to change.

You need to think about issues associated with implementing system calls. Can two different user-level processes find themselves running a system call at the same time? What data structures are private to each process, what are shared?

Note that the basic assignment does not involve implementing `fork()` (that's part of the advanced assignment). So in regard to concurrency, you can assume only a single process runs at a time. You should not synchronise any data structures you add for the basic assignment.

However, the design and implementation of your system calls should not assume only a single process will ever exist at a time. It should be possible to add a `fork()` implementation later and then add synchronisation to handle the concurrency.

While you are not restricted to only modifying these files, please place most of your implementation in the following files: function prototypes and data types for your file subsystem in `kern/include/syscall.h` or `kern/include/file.h`, and the function implementations and variable instantiations in `kern/syscall/file.c`.

Boot time initilisation code can be called from the end of `boot()` in `kern/main/main.c`.

### Notes on standard file descriptors

For any given process, the first file descriptors (0, 1, and 2) are considered to be standard input (stdin), standard output (stdout), and standard error (stderr) respectively. For this basic assignment, the file descriptors 1 (stdout) and 2 (stderr) must start out attached to the console device ("con:"), 0 (stdin) can be left unattached. You will probably modify `runprogram()` to achieve this. Your implementation must allow programs to use `dup2()` to change stdin, stdout, stderr to point elsewhere.

### Some Design Questions

- What primitive operations exist to support the transfer of data to and from kernel space?
- How will you bullet-proof the kernel from user program errors in your system calls?
- Which functions and structures need to change or be created to implement the system calls?
- How will you keep track of open files? For which system calls is this useful?
- What data structures are per-process and what structures are shared between processes?
- What are the main issues related to transferring data to and from applications?
- If `fork()` was implemented, what concurrency issues would be introduced?

For additional background, consult operating systems texts (e.g., Tanenbaum MOS, 4.4BSD, Vahalia) and the original VFS paper.

### Documenting your solution

Submit a small design document (plain text) describing the issues and your solution. Place it in `~/cs3231/asst2-src/asst2-design.txt`. Target 500–1000 words. Word wrap your document; the `fmt` command can help.

A marker should be able to answer:

- What significant data structures have you added and what function do they perform?
- What issues surround managing these structures and the state they contain?
- Which structures are per-process vs shared?
- What issues exist for transferring data to and from applications?
- If `fork()` were implemented, what concurrency issues would appear?

### FAQ, Gotchas, and Video

See the course wiki for updates and the overview video on the lectures page.

### Basic Assignment Submission

Submit via the CSE give system as a git bundle. The system will do a test build and simple execution check.

```bash
% cd ~
% give cs3231 asst2 asst2.bundle
```

Do not ignore submission results. Always push changes back to gitlab and keep your repository.

## Advanced Assignment

The advanced assignment is available for bonus marks:

- `fork(), getpid()`: 2 marks
- `waitpid(), _exit(), kill_curthread()`: 2 marks
- `exec()`: 1 mark

Complete the basic assignment plus the additional tasks below. Work on a dedicated `asst2_adv` branch.

Helpful git commands:

```bash
% git checkout -b asst2_adv
% git push --set-upstream origin asst2_adv
% git checkout master     # switch back to basic
% git checkout asst2_adv  # switch to advanced
```

### User-level Process Management System Calls

#### fork()

Implement `fork()`. Initially you can return 1 for the child pid while testing. Read comments above `mips_usermode()` and in `addrspace.h` (`as_copy`). Copy the parent trapframe carefully; consider races. `thread_fork()` may be a useful reference.

#### getpid()

Design pid allocation and reclamation. Ensure pid space does not exhaust. Then update `fork()` to return the real child pid.

#### execv(), waitpid(), _exit()

These enable multiprogramming. `fork()` creates a child. `execv()` replaces the address space and runs a new program with arguments. Manage userspace-kernel memory carefully. `waitpid()` and `_exit()` are two halves of the same mechanism; design their interaction.

#### kill_curthread()

Implement simply; do not trust any userspace state after a fatal exception. Remove the thread from the processor safely without returning to user mode.

### Design Questions

- How will arguments be passed from one user program, through the kernel, into another user program?
- How are the initial stack pointer, register contents, and return values determined?
- What new data structures will you need to manage multiple processes and what relationships do they have?
- How will you manage file accesses across multiple processes (e.g., shell reading while cat reads the same file)?
- How will you keep track of running processes?
- How will you implement `execv()` and its argument passing?

### Advanced Assignment Submission

Submission is similar to the basic assignment, but use the distinguished assignment name `asst2_adv`. Our marking scripts will switch to the `asst2_adv` branch before testing.

```bash
% cd ~
% give cs3231 asst2_adv asst2_adv.bundle
```
