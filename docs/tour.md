# Tour of Computer Systems

The compilation system/steps:

- **preprocessor**
- **compiler**
- **assembler**
- **linker**

```txt
`.c` -> preprocessor  ->  `.i`     -> compiler  ->  `.s`    -> assembler  ->  `.o`        -> linker -> executable
                         (modified                (assmebly                  (relocatable
                          source                   program)                   object
                          program)                                            programs)
```

Preprocessor reads `#` directives and insert them directly into program text.

**Buses** carry bytes of info between components.
Buses typically transfer fixed-size chunks of bytes, **words**.
Most modern machines have word sizes either 4 bytes (32-bit) or 8 bytes (64 bit).

Each I/O device is connected to the I/O bus by **controller**/**adapter**.

Main memory consists of a collection of **dynamic random access memory (DRAM)** chip.
Logically, memory is organized as a _linear_ array of bytes.

CPU contains word-size **register** called **program counter (PC)**.
PC points at (contains address of) some machine-language instruction in main memory.
Addresses pointed from PC to memory may or may not be _contiguous_.

**Instruction Set Architecture**

**Arithmetic/Logic Unit (ALU)**

**Register File** consists of a collection of word-size registers.

Operations on an instruction:

- **load**: byte/word copied register ← main memory
- **store**: byte/word register → main memory
- **operate**:
  - copy contents from _two_ registers to **ALU**
  - perform arithmetic operation on the two words
  - store the result in register
- **jump**:
  - extract word from the instruction itself
  - copy word into PC

**Direct Memory Access (DMA)**: data (executable file) travel directly from disk to memory, without passing through processor.

When the program is loaded, its executable gets copied to main memory.
When program is run by processor, its instructions get copied from memory to processor.

Processor can read from register 100 times faster than memory.

**Cache Memory**

- L1 - nearly as fast as register
- L2
  - larger
  - connected to processor by a special bus
  - might take 5 times longer to access L2 than L1

L1 and L2 are implemented with **static random access memory (SRAM)**.
Even L3.

**locality**

Processes run **concurrently**.
**Concurrent**: instructions of one process are interleaved with those of another process.

**Content Switching**

**kernel**

- manages transition from one process to another
- _always_ resident in memory

**Threads**

- a process can contain multiple threads
- each sharing the same context of the process
- sharing same code and global data
- typically more efficient than processes

**Virtual Memory**

- **virtual address space**
- **stack**
  - at top of user's virtual address space
  - used by compiler to implement function calls
- store the contents of a processor's virtual memory on disk and then use memory as a cache for the disk

Each L1 cache is split into 2 parts:

- holds recently fetched instructions
- holds data

**Hyperthreading**

**Instruction-Level Parallelism**

**Single-Instruction, Multiple-Data (SIMD)**

- allows a single instruction to cause multiple operations to be performed in parallel
- for processing image/sound/video data
