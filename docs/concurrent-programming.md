# Concurrent Programming

## Concurrent Programming with Processes

- with `fork`/`exec`/`waitpid`

### Concurrent Server based on Processes

- any real world concurrent server must:
  - include `SIGCHLD` handler
  - be prepared to reap multiple zombie children
    - `SIGCHLD` signals are blocked while the `SIGCHLD` handler is executing
    - Linux signals are _NOT_ queued

## Concurrent Programming with I/O Multiplexing

- use `select` function to ask kernel to suspend the process,
  - returning control to the application _only after_ one or more I/O events occurred
- `int select(int n, fd_set *fdset, NULL, NULL, NULL);`
- `select` manipulates **descriptor sets**
  - of type `fd_set`
- macros to _inspect_ a descriptor set:
  - `FD_ZERO`
  - `FD_SET`
  - `FD_CLR`
  - `FD_ISSET`

## With Threads

- **thread**
  - a **logical** flow running in context of a process
  - scheduled _automatically_ by kernel
  - each thread has its own **thread context**
    - a unique integer **thread ID** (TID)
    - stack
    - stack pointer
    - program counter
    - general purpose registers
    - condition codes
  - _all_ threads running in a process share the entire virtual address space of the process
    - code
    - data
    - heap
    - shared libs
    - open files
- thread context switch _faster_ than process context switch

### POSIX Threads

- **pthread**
- **thread routine**
- `int pthread_create(pthread_t *tid, pthread_attr_t *attr, func *f, void *arg);`
  - runs the thread routine `f` in the context of the newly created thread
- `pthread_t pthread_self(void);`
  - new thread can determine its own TID
- `int pthread_join(pthread_t tid, void **thread_return);`
  - _blocks_ until thread `tid` terminates
  - assign the generic `void *` pointer returned by the **thread routine** to the location pointed to by `thread_return`
  - _reaps_ any memory resources held by the terminated thread
  - main thread waits for the peer to terminate
- `pthread_exit(void *thread_return);`
  - explicitly terminates
- so apparently `exit()` terminates the process
- `int pthread_cancel(pthread_t tid);`
  - some other peer terminates the current thread
- at any point in time, a thread is **joinable** or **detached**
  - by default threads are created **joinable**
- a **detached** thread _cannot_ be reaped/killed by other threads
  - memory resources freed _automatically_ by system
- `int pthread_detach(pthread_t tid);`
  - threads detach themselves by `pthread_detach(pthread_self())`
- initializing threads

  ```c
  pthread_once_t once_control = PTHREAD_ONCE_INIT;
  int pthread_once(pthread_once_t * once_control, void (*init_routine)(void));`
  ```

  - initialize state associated with a thread routine

-
