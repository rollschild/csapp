# Exceptional Control Flow

- **flow of control**: control transfers from one instruction to the next
- **ECF** (exceptional control flow)
  - the basic mechanism OS uses to implement I/O, processes, and virtual memory
  - **trap**
  - **system call**
  - basic mechanism for implementing concurrency
  - non-local jumps (`setjmp` & `longjmp`)

## Exceptions

- **exception table**: entry `k` contains address of handler for exception `k`
  - starting address contained in special CPU register **exception table base register**
- Exception numbers:
  - index into the **exception table**
  - unique non-negative integer
  - assigned by processor:
    - divide by zero
    - page faults
    - memory access violations
    - break points
    - arithmetic overflows
  - assigned by kernel:
    - system calls
    - signals from I/O devices
- exception handlers run in **kernel mode**
  - having complete access to all system resources

### Classes of Exceptions

- Four classes:
  - interrupts
  - traps
  - faults
  - aborts
- Interrupts
  - _asynchronously_
  - result of signals from I/O devices
  - interrupt pin has gone high
- Traps
  - _intentional_
  - _synchronously_
  - occur as result of executing an instruction
  - return control to next instruction
  - most important use: **system call**
    - `fork`/`execve`/`exit`/`read`
    - run in **kernel mode**
- Linux/x86-64 faults & aborts
  - 2556 different types
  - 0 - 31: Intel defined
  - 32 - 255: interrupts/traps defined by OS
  - **general protection fault**
    - program referencing undefined area of virtual memory
    - program attempting to write to a read-only text segment
    - not recoverable by Linux
    - **segmentation faults**
- Linux/x86-64 system calls
  - can be called directly in C via `syscall`
  - _ALL_ args to Linux system calls are passed through general-purpose registers _rather than_ stack
  - `%rax` - syscall number
  - up to six arguments in:
    - `%rdi`
    - `%rsi`
    - `%rdx`
    - `%r10`
    - `%r8`
    - `%r9`
  - on return from system call,
    - `%rcx` & `%r11` are _destroyed_
    - `%rax` contains return value
    - `errno` contains negative value between `-4095` and `-1` if error

## Processes

- **process**: an instance of a program in execution
- each program in the system runs in the **context** of some process

### Logical Control Flow

- each process executes a _portion_ of its flow and then is **preempted**,
  - while other processes take their turns

### Concurrent Flows

- a logical flow whose execution overlaps _in time_ with another flow
- **multitasking** & **time slicing**
- **parallel flows**
  - if two flows are running concurrently on different processor cores or computers
  - **parallel execution**

### Private Address Space

- **address space**
- code segment _always_ starts at address `0x400000`

### User and Kernel Modes

- **mode bit**
  - in some control register
  - when set, run in **kernel mode**
  - when not set, run in **user mode**
    - must access kernel code & data via system call interface
- the _ONLY_ way for process to change from user mode to kernel mode:
  - via exception:
    - interrupt
    - fault
    - trapping system call
- the `/proc` filesystem on Linux
  - allows user mode processes to access contents of kernel data structures
  - `/proc/cpuinfo`
  - `/proc/<process-id>/maps` - memory segment used by a particular process
- `/sys` filesystem
  - additional low-level info about system buses & devices

### Context Switches

- kernel maintains a **context** for each process
- **context** - the state that the kernel needs to restart a preempted process
  - general-purpose registers
  - kernel's stack
  - page table - characterizes current process
  - process table - info about current process
  - file table - info about files the process has opened
- **scheduling**
  - kernel decides to preempt the current process and restart a previously preempted process
  - **scheduler**
- all systems have some mechanism for generating periodic timer interrupts
  - typically every 1ms or 10ms
  - when kernel can decide whether the current process has run long enough

## System Call Error Handling

- Unix system-level functions typically return -1 when error occurs then set `errno`
  - `strerror(errno)`
- **error-handling** wrappers

## Process Control

- `getpid` vs. `getppid`
- Three states of a process:
  - **running**
  - **stopped**
    - result of receiving:
      - `SIGSTOP`
      - `SIGTSTP`
      - `SIGTTIN`
      - `SIGTTOU`
    - remains stopped until receiving `SIGCONT`
- To create a new process, `fork()`

  ```c
  pid_t fork(void);
  ```

  - child process gets:
    - identical (but separate) copy of parent's user-level virtual address space
    - identical copies of any of parent's open file descriptors
      - child can read/write any files that were open in parent when `fork` called

- `fork` function returns _twice_:
  - once in the calling process (parent)
    - returns PID of the child
  - once in the newly created child process
    - return value of `0`

```c
// this creates *4* processes!!!
int main() {
    fork();
    fork();
    printf("hello\n");
    exit(0);
}
```

### Reaping Child Processes

- when terminated, process is kept around in a **terminated** state until it's **reaped** by parent
- **zombie** - a terminated process that has _NOT_ yet been reaped
- `init` process
  - adopted parent of any orphaned children
  - PID of `1`
  - created by kernel during system startup
  - _NEVER_ terminates
  - ancestor of _every_ process
  - reap a parent process's zombie children if the parent process terminates without reaping its own children
- zombies still consume system memory resources even though not running
- a process waits for its children to terminate/stop by calling `waitpid`
  - `pid_t waitpid(pid_t pid, int *statusp, int options);`
  - `options`: default `0` - _suspends_ execution of caller until a child process in its **wait set** terminates
    - `WNOHANG`
    - `WUNTRACED`
    - `WCONTINUED`
    - `WNOHANG | WUNTRACED`
  - `statusp`
    - `WIFEXITED(status)`
    - `WEXITSTATUS(status)`
    - `WIFSIGNALED(status)`
    - `WTERMSIG(status)`
    - `WIFSTOPPED(status)`
    - `WSTOPSIG(status)`
    - `WIFCONTINUED(status)`
- `wait` - simpler version of `waitpid`
  - `pid_t wait(int *statusp);`
  - calling `wait(&status)` === `waitpid(-1, &status, 0)`
- reaping children in the same order they were created by the parent:

  ```c
  #include <sys/wait.h>
  #include <errno.h>

  #define N 13

  int main() {
      int status, i;
      pid_t pid[N], retpid;

      // parent creates N children
      for (i = 0; i < N; i++) {
          if ((pid[i] = fork()) == 0) /* child */
              exit(100 + i);
      }

      // parent reaps N children in order
      i = 0;
      while ((retpid = waitpid(pid[i++], &status, 0)) > 0) {
          if (WIFEXITED(status)) {
              printf("child %d terminated normally with exit status=%d\n",
                      retpid, WEXITSTATUS(status));
          } else {
              printf("child %d terminated abnormally\n", retpid);
          }
      }

      // the only normal termination is if there are no more children
      if (errno != ECHILD) {
          fprintf(stderr, "%s: %s\n", "waitpid error", strerror(errno));
          exit(0);
      }
  }
  ```

### Putting Processes to Sleep

- `unsigned int sleep(unsigned int secs);`
  - _could_ return non-`0` values
  - interrupted by signal
- `int pause(void)` - puts the calling function to _sleep_ until signal received

### Loading & Running Programs

- `int execve(const char* filename, const char *argv[], const char *envp[]);`
  - called _once_
  - _never_ returns (unless error)
  - `argv` - `NULL`-terminated array of pointers
  - `envp`
    - `NULL`-terminated array of pointers
    - `name=value` pairs
- main routine of the loaded program:
  - `int main(int argc, char **argv, char **envp);`
  - `int main(int argc, char *argv[], char *envp[]);`
- system start-up function, `libc_start_main`
- `char *getenv(const char *name);`
- `int setenv(const char *name, const char *newvalue, int overwrite);`
- `void unsetenv(const char *name);`

### Use `fork` and `execve` to run programs

- shell performs a sequence of **read**/**evaluate** steps then _terminates_
- `fork` vs. `execve`:
  - `fork` runs the same program in a _new_ child process that is a _duplicate_ of the parent
  - `execve` loads & runs a new program in the context of the _current_ process
    - _overwrites_ the address space of the current process
    - but does _NOT_ create a new process
    - new process has _same_ PID

```c
/*
 * shellex.c
 */
#include "stdio.h"
#include <sys/wait.h>
#include <errno.h>

#define MAXARGS 128

// function prototypes
void eval(char *cmdline);
int parseline(char *buf, char **argv);
int builtin_command(char **argv);

int main() {
    char cmdline[MAXLINE]; // command line
     while (1) {
        // read
        printf("> ");
        fgets(cmdline, MAXLINE, stdin);
        if (feof(stdin)) {
            exit(0);
        }

        // evaluate
        eval(cmdline);
     }
}

void eval(char *cmdline) {
    char *argv[MAXARGS];
    char buf[MAXLINE];
    int bg;
    pid_t pid;

    strcpy(buf, cmdline);
    bg = parseline(buf, argv);
    if (argv[0] == NULL) {
        return;
    }

    if (!builtin_command(argv)) {
        if ((pid = fork()) == 0) {
            // child runs user job
            if (execve(argv[0], argv, environ) < 0) {
                printf("%s: command not found!\n", argv[0]);
                exit(0);
            }
        }

        // parent waits for foreground job to terminate
        if (!bg) {
            int status;
            if (waitpid(pid, &status, 0) < 0) {
                fprintf(stderr, "%s: %s\n", "waitfg: waitpid error", strerror(errno));
                exit(0);
            }
        } else {
            printf("%d %s", pid, cmdline);
        }
    }

    return;
}

int builtin_command(char **argv) {
    if (!strcmp(argv[0], "quit")) {
        // quit command
        exit(0);
    }
    if (!strcmp(argv[0], "&")) {
        // ignore &
        return 1;
    }
    return 0;
}

int parseline(char *buf, char **argv) {
    char *delim; // points to first space delimiter
    int argc;
    int bg;

    buf[strlen(buf) - 1] = ' '; // replace trailing '\n' with space
    while (*buf && (*buf == ' ')) {
        // ignore leading space
        buf++;
    }

    // build argv list
    argc = 0;
    while ((delim = strchr(buf, ' '))) {
        argv[argc++] = buf;
        *delim = '\0';
        buf = delim + 1;
        while (*buf && (*buf == ' ')) {
            // ignore leading space
            buf++;
        }
    }

    argv[argc] = NULL;

    if (argc == 0) {
        // ignore blank line
        return 1;
    }

    if ((bg = (*argv[argc - 1] == '&')) != 0) {
        argv[--argc] = NULL;
    }

    return bg;

}
```
