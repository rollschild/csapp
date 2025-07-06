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

## Shared Variables in Threaded Programs

- registers are _NEVER_ shared
- virtual memory is _ALWAYS_ shared
- thread stacks are _usually_ accessed independently by their respective threads
  - but _NOT_ always
- different thread stacks are _NOT_ protected from other threads

### Mapping Variables to Memory

- global variables
  - at run time, exactly one instance of each global variable
  - can be referenced by _any_ thread
- local automatic variables
  - declared inside a function without `static` attribute
  - at run time, each thread's stack contains its own instances of any local automatic
  - true even if multiple threads execute the same thread routine
- local static variables
  - declared inside a function with `static`
  - the read/write area of virtual memory contains exactly one instance of each
- **shared variable**
  - _if and only if_ one of its instances is referenced by more than one thread

## Synchronizing Threads with Semaphores

- **semaphore**
  - global variable with a non-negative integer value
  - can only be manipulated by 2 special ops:
    - `P(s)`
      - if `s` is nonzero, `P` decreases `s` and returns immediately
      - if `s` is zero, then suspend the thread until `s` becomes nonzero and thread is restarted by `V` op
      - after restarting, `P` decrements `s` and returns control to caller
    - `V(s)`
      - increments `s` by `1`
      - if any threads blocked at `P` op waiting for `s` to become nonzero, then `V` op restarts _exactly one of_ these threads
        - which then completes its `P` op by decrementing `s`
- **semaphore invariant**
  - a running program can _never_ enter a state where a properly initialized semaphore has a negative value
- **binary semaphore**
  - associate a semaphore `s`, initially `1`, with each shared variable (or related set of shared variables) and then surround the corresponding critical section with `P(s)` and `V(s)` ops
  - **mutex**
  - `P(s)` - **lock** a mutex
  - `V(s)` - **unlock** a mutex

### Using Semaphores to Schedule Shared Resources

- **producer-consumer** & **readers-writers**

#### Producer-Consumer Problem

- a **bounded buffer** shared by a producer and a consumer thread

#### Readers-Writers Problem

#### Readers-Writers Problem

```c
/**
Favors readers over writers
*/
// global variables
int readcnt; // number of readers currently in the critical section
// both below initially = 1
sem_t mutex;
// controls access to the critical sections that access the shared object
sem_t w;

void reader(void) {
    // as long as a single reader holds the w mutex, an unbounded number of
    // readers can enter the critical section unimpeded
    while (1) {
        P(&mutex); // protects access to readcnt
        readcnt++;
        if (readcnt == 1) { /* first in */
            // only the first reader to enter critical section locks w
            P(&w);
        }
        V(&mutex);

        /* critical section */
        /* reading happens */

        P(&mutex);
        readcnt--;
        if (readcnt == 0) { /* last out */
            // only the last reader to leave critical section unlocks w
            V(&w);
        }
        V(&mutex);
    }
}

void writer(void) {
    while (1) {
        P(&w);

        /* critical section */
        /* writing happens */

        V(&w);
    }
}
```
