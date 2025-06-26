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

## Reading File Metadata

- `int stat(const char *filename, struct stat *buf);`
- `int fstat(int fd, struct stat *buf);`

## Reading Directory Contents

- `DIR *opendir(const char *name);`
- `struct dirent *readdir(DIR *dirp);`
- `int closedir(DIR *dirp);`

## Sharing Files

- **descriptor table**
- **file table**
- **v-node table**
- after `fork`, child has its own _duplicate_ copy of the parent's descriptor table
  - parent & child share the same set of open file tables and thus share the same file position

## I/O Redirection

- `int dup2(int oldfd, int newfd);`
  - copies descriptor table entry `oldfd` to descriptor table entry `newfd`
  - overwriting previous contents of descriptor table entry `newfd`
  - if `newfd` already open, `dup2` _closes_ `newfd` before copying `oldfd`

## Standard I/O

- standard I/O lib models an open file as **stream**
- a stream of type `FILE` is an abstraction for a file descriptor and a **stream buffer**

## Considerations

- standard I/O streams are **full duplex**
  - programs can perform input & output on the _same_ stream
- RESTRICTIONS:
  - input functions following output functions
    - an input function cannot follow an output function without an intervening call to:
      - `fflush`
      - `fseek`
      - `fsetpos`
      - `rewind`
  - output functions following input functions
    - an output function cannot follow an input function without intervening call to:
      - `fseek`
      - `fsetpos`
      - `rewind`
    - unless the input function encounters EOF
-
