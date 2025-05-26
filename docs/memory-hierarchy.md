# The Memory Hierarchy

- **locality**

## Storage Techs

### Random Access Memory

- **SRAM (static RAM)**
  - for cache memories
  - both _on_ and _off_ CPU chip
  - more expensive
  - **bistable**
- **DRAM (dynamic RAM)**
  - main memory
  - frame buffer of a graphic system
  - stores each bit as charge on a capacitor
  - _very_ sensitive to any disturbance
- **memory controller**
- **VRAM**
  - allows concurrent reads & writes to the memory
- Nonvolatile Memory

### Disk

- takes _million times_ longer than reading from SRAM
- Logical Disk Blocks
  - **disk controller**

## Locality

- Reference data items that:
  - are near other recently referenced data items, or
  - were recently referenced themselves
- Two distinct forms:
  - **temporal locality**
  - **spatial locality**
- **cache memory**

## Cache Memories

- **L1 Cache**
  - small SRAM **cache memory**
  - between CPU register file and main memory
  - _nearly_ as fast as registers
  - typically in about 4 clock cycles

### Direct-mapped cache

- **direct-mapped cache**: cache with exactly one line per set (E = 1)
- ## **Conflict Misses** in Direct-Mapped Caches

### Set Associative Caches

### Fully Associative Caches

- a single set that contains all cache lines

### Issues with Writes

- **write-through** vs. **write-back**
- **write-allocate** vs. **no-write-allocate**
- Recommendation:
  - assumes write-back, write-allocate caches
- **virtual memory systems** use **write-back** exclusively

### Anatomy of Real Cache Hierarchy

- caches can hold instructions as well as data
- **i-cache** - cache that holds instructions _only_
  - _typically_ **read-only**
- **d-cache** - cache that holds program data _only_
- **unified cache** - cache that holds both instructions and data

### Performance Impact of Cache Parameters

- **miss rate**
- **hit rate**
- **hit time** - several clock cycles for L1 caches
- **miss penalty**
  - penalty for L1 misses served from L2 is on the order of 10 cycles
  - from L3, 50 cycles
  - from main memory, 200 cycles
- _harder_ to make larger memories run faster
  - larger caches -> worse hit time
- larger blocks have _negative_ impact on miss penalty
  - larger transfer times
- Core i7 cache blocks contain 64 bytes
- Higher associativity - larger values of number of cache lines per set

## Writing Cache-Friendly Code

- Basic approach:
  - make common case go fast
  - minimize number of cache misses in each inner loop
- repeated references to local variables are good because
  - compiler can cache them in the register file - **temporal locality**
- **stride-1**reference patterns are good because
  - caches at all levels of the memory hierarchy store data as contiguous blocks
  - **spatial locality**

## Impact of Caches on Program Performance

- **read throughput**
  - a.k.a. **read bandwidth**
- hardware **prefetching**
- The aim is:
  - to exploit temporal locality so that heavily used words are fetched from the L1 cache
  - to exploit spatial locality so that as many words as possible are accessed from a single L1 cache line
- **blocking**
  - organize data structures in a program into large chunks called **blocks**
  - it loads a chunk into L1 cache, does all reads/writes on the chunk, then discards the chunk, then loads next chunk
  - can improve temporal locality of inner loops

### Exploiting Locality in Programs

- Focus on the inner loops!
- Try to max spatial locality by reading data objects _sequentially_, with stride 1, in the order they are stored in memory
- Try to max temporal locality by using data object as often as possible once it has been read from memory
