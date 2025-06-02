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
