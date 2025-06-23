# System-Level I/O

## Unix I/O

- **descriptor**
- _each_ process created by Linux shell begins with 3 _open_ files:
  - `stdin` - descriptor `0`
    - `STDIN_FILENO`
  - `stdout` - descriptor `1`
    - `STDOUT_FILENO`
  - `stderr` - descriptor `2`
    - `STDERR_FILENO`
- **file position** `k` - maintained by kernel
  - byte offset from beginning of a file
- `read`
  - copies `n` > 0 bytes from a file to _memory_
  - starting at position `k`, then incrementing `k` by `n`
- when a process terminates for _any_ reason, kernel closes _all_ open files and frees their memory resources

## Files

- **directory**
  - file consisting of array of **links**
  - link maps a filename to a file
- file types:
  - **named pipes**
  - **symbolic links**
  - **character**
  - **block devices**

## Opening & Closing Files

- each process has `umask`
- `int open(char *filename, int flags, mode_t mode);`
  - access permission bits of the file: `mode & ~umask`
