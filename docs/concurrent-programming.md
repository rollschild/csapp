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
-
