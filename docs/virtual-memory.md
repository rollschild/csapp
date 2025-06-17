# Virtual Memory

- Three capabilities
  - uses main memory efficiently by treating it as a _cache_ for an address space stored on disk
    - keeping only the active areas in main memory, and
    - transferring data back and forth between disk and memory as needed
  - simplifies memory management by providing each process with a uniform address space
  - protects address space of each process from corruption by other processes

## Physical & Virtual Addressing

- **physical address**
- **virtual address**
  - **address translation**
  - **memory management unit (MMU)** - dedicated hardware on CPU

## Address Spaces

- n-bit address space
  - number of bits to represent its largest address
  - 32- or 64-bit address space

## VM as Tool for Caching

- as an array of N contiguous byte-size cells stored _on disk_
- each byte has unique address that serves as index into the array
- **virtual pages**
  - fixed-size blocks
  - each is $P = 2^p$ bytes in size
- three disjoint sets any time:
  - **unallocated**
    - do _NOT_ occupy space on disk
  - **cached**
    - in physical memory
  - **uncached**
- **DRAM cache**
  - Virtual memory system's cache that caches virtual pages in main memory
- cost of reading 1st byte from a disk sector is ~100,000 times slower than reading successive bytes in the sector
- fully **associative**: _any_ virtual page can be placed in _any_ physical page
- DRAM caches _always_ use **write-back** instead of write-through
- **page table**
  - data structure stored in physical memory
  - maps virtual pages to physical pages
  - MMU reads it every time
- **page fault**
  - a DRAM cache miss
  - **swapped in**/**swapped out**
- **demand paging**
  - waiting until the last moment to swap in a page, when a miss occurs
  - all modern systems
- allocating pages
  - `malloc`
- **locality**
  - **temporal locality**
  - **thrashing**
    - pages are swapped in n out _continuously_

## VM as Tool for Memory Management

- OS provides a _separate_ page table
  - separate virtual address space for each process

## Address Translation

### Multi-Level Page Tables

- hierarchy of page tables

## Intel Core i7/Linux Memory System

- each process has its own private page table hierarchy
- when a linux process running, the page tables associated with allocated pages are all memory-resident

### Linux Virtual Memory System

- **areas** (**segments**)
  - contiguous chunk of existing (allocated) virtual memory whose pages are related in some way
- `task_struct`
  - a distinct data structure maintained by kernel
  - for each process in system
- `pgd`
  - page global directory
  - stored in CR3 control register?
- `mmap` - points to `vm_area_structs`

## Memory Mapping

- **memory mapping**
  - linux initializes contents of a virtual memory area by associating it with an **object** on disk
  - areas can be mapped to one of 2 types of objects:
    - regular file in Linux file system
    - anonymous file
      - containing binary zeros
      - **demand-zero pages**
- **swap file**
  - created by kernel
  - bounds the total amount of virtual pages that can be allocated by currently running processes

### Shared Objects

- **copy-on-write**
  - how private objects are mapped into virtual memory
  - protection fault triggered
  - fault handler
    - creates new copy of the page in physical memory
    - updates page table entry to point to the new copy
    - then restores write permissions to the page

### `fork`

- kernel creates various data structures for the new process and assigns it a new PID
- To create virtual memory for the new process:
  - creates exact copies of current process's:
    - `mm_struct`
    - area structs
    - page tables
  - flags each page in both processes as read-only
  - flags each area struct in both processes as private copy-on-write

### `execve`

- Steps to load & run `a.out`:

  1. delete existing user areas (of the current process)
  2. map private areas
  3. map shared areas
  4. set program counter (PC)

### User-Level Memory Mapping with `mmap`

- `mmap`: used by linux processes to create new areas of virtual memory and to map objects into these areas
- `void *mmap(void *start, size_t length, int prot, int flags, int fd, off_t offset)`;
- asks kernel to:
  - create new virtual memory area
    - _preferably_ starting at address `start`
  - map a contiguous chunk of object specified by file descriptor `fd` to the new area
    - with `length` bytes
    - starts at offset of `offset` bytes from beginning of the file
  - `start` is just a hint - usually `NULL`
  - `prot`: bits describing access permissions of the newly mapped virtual memory area
    - `PROT_EXEC`
    - `PROT_READ`
    - `PROT_WRITE`
    - `PROT_NONE`
- `int munmap(void *start, size_t length);`
  - delete the area
