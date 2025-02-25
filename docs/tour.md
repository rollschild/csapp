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
