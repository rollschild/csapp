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

## Signals

- **signal**: _small_ message that notifies a process that an event of some type has occurred in the system
- `SIGFPE`: divide by zero
- `SIGSEGV` - illegal memory reference
- `SIGINT` - Ctrl+C
- when child process terminates/stops, kernel sends `SIGCHLD` to parent

### Signal Terminology

- the receiving process can:
  - ignore the signal,
  - terminate, or
  - catch the signal by executing user-level **signal handler**
- **pending signal**
  - at _any_ point in time, there can be _at most one_ pending signal _of a particular type_
  - in `pending` bit vector of each process
- **blocked signal**
  - in `blocked` bit vector

### Sending Signals

- **process group**
  - `getpgrp`
    - returns process group ID
  - `int setpgid(pid_t pid, pid_t pgid);`
- `/bin/kill`
- Typing Ctrl+C:
  - causes kernel to send a `SIGINT` signal to _every_ process in the foreground process group
- Typing Ctrl+Z:
  - causes kernel to send a `SIGTSTP` signal to _every_ process in the foreground process group
- `int kill(pid_t pid, int sig);`
- `unsigned int alarm(unsigned int secs);`
  - kernel to send `SIGALRM` to the _calling_ process in `secs` seconds

### Receiving Signals

- when kernel switches a process `p` from kernel mode to user mode (returning from a system call or completing a context switch)
  - it checks set of _unblocked_ _pending_ signals (`pending & ~blocked`)
  - if the set is empty (usual case), kernel passes control to the next instruction in the logical control flow of `p`
  - if set if _not_ empty, kernel chooses some signal `k` in the set (typically the smallest `k`) and forces `p` to _receive_ signal `k`
- each signal type has a predefined **default action**, one of the following:
  - terminates
  - terminates and dumps core
  - stops/suspends until restarted by `SIGCONT`
  - ignores signal
- `sighandler_t signal(int signum, sighandler_t handler);`
  - default actions of `SIGSTOP` and `SIGKILL` _CANNOT_ be changed

### Blocking & Unblocking Signals

- **implicit blocking mechanism**
- **explicit blocking mechanism**
  - `int sigprocmask(int how, const sigset_t *set, sigset_t *oldset);`

### Writing Signal Handlers

- _keep handles as simple as possible_
- call only _async-signal-safe_ functions in the handler
  - _NOT_ async-signal-safe:
    - `printf`
    - `sprintf`
    - `malloc`
    - `exit`
- `write` - the _ONLY_ safe way to generate output from a signal handler
- save then restore `errno`
- _declare global variables with `volatile`_
  - `volatile int g;`
    - tells compiler _NOT_ to cache a variable
    - forces compiler to read value of `g` from memory each time
  - each access to global variable should be protected by temporarily blocking signals
- _declare flags with `sig_atomic_t`_
  - handler records the receipt of signal by writing to global **flag**
  - can be implemented: `volatile sig_atomic_t flag;`
  - can safely read from and write to `sig_atomic_t` variables without temporarily block signals
  - _ONLY_ applies to individual reads/writes
  - _NOT_ applicable to `flag++` or `flag = flag + 10`
- _signals CANNOT be used to count the occurrence of events in other processes_

```c
// signal handler
void handler(int sig) {
    int old_errno = errno;
    // reap as many zombie children as possible each time the handler is invoked
    while (waitpid(-1, NULL, 0) > 0) {
        sio_puts("handler reaped child\n");
    }

    if (errno != ECHILD) {
        sio_error("waitpid_error");
    }

    sleep(1);
    errno = old_errno;
}
```

- system calls can be interrupted
- Posix defines `sigaction`,
  - allows users to clearly specify the signal-handling semantics they want when they install a handler
  - `int sigaction(int signum, struct sigaction *act, struct sigaction *oldact);`

```c
/*
 * Signal, wrapper function for `sigaction` that provides portable signal
 * handling on Posix-compliant systems.
 *   - only signals of the type currently being processed by handler are blocked
 *   - signals are _NOT_ queued
 *   - interrupted system calls are automatically restarted whenever possible
 *   - once sig handler installed, it remains installed until `Signal` is called
 *     with a `handler` arg of either `SIG_IGN` or `SIG_DFL`
 * By W. Richard Stevens
 */
handler_t *Signal(int signum, handler_t *handler) {
    struct sigaction action, old_action;

    action.sa_handler = handler;
    sigemptyset(&action.sa_mask); // blocks sigs of type being handled
    action.sa_flags = SA_RESTART; // restart syscalls if possible

    if (sigaction(signum, &action, &old_action) < 0) {
        fprintf(stderr, "%s: %s\n", "Signal error", strerror(errno));
        exit(0);
    }

    return (old_action.sa_handler);
}
```

### Synchronizing flows to avoid nasty concurrency bugs

- **race**

```c
void handler(int sig) {
    int old_errno = errno;
    sigset_t mask_all, prev_all;
    pid_t pid;

    Sigfillset(&mask_all);
    while ((pid = waitpid(-1, NULL, 0)) > 0) {
        // reap a zombie child
        Sigprocmask(SIG_BLOCK, &mask_all, &prev_all);
        delete_job(pid); // delete child from the job list
        Sigprocmask(SIG_SETMASK, &prev_all, NULL);
    }

    if(errno != ECHILD) {
        Sio_error("waitpid error");
    }

    errno = old_errno;
}

/**
 * Eliminate race by blocking SIGCHLD signals before the call to `fork`
 * and then unblocking them _only_ after we have called `add_job`.
 * Guarantee that the child will be reaped _after_ being added to job list
 */
int main(int argc, char **argv) {
    int pid;
    sigset_t mask_all, mask_one, prev_one;

    Sigfillset(&mask_all);
    Sigemptyset(&mask_one);
    Sigaddset(&mask_one, SIGCHLD);
    Signal(SIGCHLD, handler);
    init_jobs(); // initialize job list

    while (1) {
        Sigprocmask(SIG_BLOCK, &mask_one, &prev_one); // block SIGCHLD
        if ((pid = fork()) == 0) {
            // child process
            // children _INHERIT_ the `blocked` set of their parents
            // we must unblock SIG_CHLD in the child
            Sigprocmask(SIG_SETMASK, &prev_one, NULL); // unblock SIGCHLD
            execve("/bin/date", argv, NULL);
        }
        Sigprocmask(SIG_BLOCK, &mask_all, NULL); // parent process
        add_job(pid); // add job to job list
        Sigprocmask(SIG_SETMASK, &prev_one, NULL); // unblock SIGCHLD
    }

    exit(0);
}
```

### Explicitly Waiting for Signals

```c
#include "stdio.h"
#include <sys/wait.h>
#include <errno.h>

volatile sig_atomic_t pid;

void sigchld_handler(int s) {
    int old_errno = errno;
    // after child terminates, reap it and assign its non-zero PID to global pid
    pid = waitpid(-1, NULL, 0);
    errno = old_errno;
}

void sigint_handler(int s) {
}

/**
 * Explicitly wait for a certain signal handler to run.
 * E.g. when linux shell creates a foreground job, it must wait for the job to
 * terminate and be reaped by `SIGCHLD` handler before accepting the next user
 * command
 */
int main(int argc, char **argv) {
    sigset_t mask, prev;

    Signal(SIGCHLD, sigchld_handler);
    Signal(SIGINT, sigint_handler);
    Sigemptyset(&mask);
    Sigaddset(&mask, SIGCHLD);

    // Before each call to sigsuspend, SIGCHLD is blocked;
    // sigsuspend temporarily unblocks SIGCHLD, then sleeps until the parent
    // catches a signal.
    // Before returning, it restores the original blocked set, which blocks
    // SIGCHLD again.
    // If parent caught a SIGINT, then loop test succeeds and the next iteration
    // calls `sigsuspend` again.
    // If parent caught SIGCHLD, loop test fails then exits the loop.
    while (1) {
        Sigprocmask(SIG_BLOCK, &mask, &prev); // block SIGCHLD
        if (fork() == 0) {
            // child
            exit(0);
        }

        // wait for SIGCHLD to be received
        pid = 0;

        while (!pid) {
            sigsuspend(&prev);
        }

        // optionally unblock SIGCHLD
        Sigprocmask(SIG_SETMASK, &prev, NULL);

        // do some work after receiving SIGCHLD
        printf(".");
    }

    exit(0);
}
```

- `int sigsuspend(const sigset_t *mask);`
  - temporarily replaces the current blocked set with `mask`
  - then suspends the process until receipt of a signal whose actions is either to run a handler or to terminate the process
  - equivalent to an **atomic** (uninterruptible) version of:
    ```c
    sigprocmask(SIG_BLOCK, &mask, &prev);
    pause();
    sigprocmask(SIG_SETMASK, &prev, NULL);
    ```
-
