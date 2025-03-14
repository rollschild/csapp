# Machine-Level Representation of Programs

## Program Encodings

- **object-code** files
  - by assembler
  - contain binary representations of all instructions,
  - but the addresses of global values are _NOT_ filled in yet

### Machine-Level Code

- **program counter**
  - `%rip`
  - address in memory of the _next_ instruction to be executed
- **register file**
  - 16 named locations
  - storing 64-values each
- **condition code register**
- **vector register**
- x86-64 virtual addresses are represented by 64-bit words
- `pushq %rbx` - push contents of `%rbx` onto program stack
- use **disassembler** to examine machine-code bytes
  - `objdump` on Linux
  - `objdump -d <name>.o`
- _only_ `pushq %rbx` can start with value `53`
- one of the set of object-code files _must_ contain a `main` function
- linker
  - matches functions calls with the locations of the executable code for those functions
- assembly lines starting with `.` (like `.file`, `.ident`, and `.section`) are directives to guide the assembler and linker
- ATT vs Intel format
  - operands in reverse order

## Data Types

- **word**
  - 16 bits
- In C:
  - `short` - word size - 2 bytes
- `movw` - move **word** - 2 bytes
- `movl` - move **double word** - 4 bytes
  - `int`
- `movq` - move ** quad word** - 8 bytes
  - `long`
  - `char*`

## Accessing Info

- x86-64 CPU contains a set of 16 **general-purpose** **registers**, storing 64-bit values each
  - `%rax` - return value
  - `%rbx`
  - `%rcx`
  - `%rdx`
  - `%rsi`
  - `%rdi`
  - `%rbp`
  - `%rsp` - stack pointer
    - end position of the runtime stack
  - `%r8`
  - `%r9`
  - `%r10`
  - `%r11`
  - `%r12`
  - `%r13`
  - `%r14`
  - `%r15`
- Originally, on 8086, `%ax` through `bp`, 8 16-bit registers
- On IA32, extended to `%eax` - `%ebp`
- Instructions can operate on data of different sizes stored in the low-order bytes of the 16 registers
- Conventions:
  - instructions that generate 1- or 2-byte quantities leave the remaining bytes unchanged
  - instructions that generate 4-byte quantities set upper 4 bytes of the register to zero
    - IA32 -> x86-64

### Operand Specifiers

- **immediate** - constant values
  - `$Imm` (ATT)
- register
  - `r_a` - a register
  - `R[r_a]` - its value
  - contents of the register
- **memory reference**
  - `Imm(r_b, r_i, s)` - most general form
    - effective address: `Imm + R[r_b] + R[r_i] * s`
    - where `s` must be 1, 2, 4, or 8
    - useful when referencing array/structure elements

### Data Movement Instructions

- `movb`/`movw`/`movl`/`movq`
- `  1  /   2  /   4  /   8     bytes`
- `movabsq` - move absolute quad word
- move instruction _CANNOT_ have both operands to be memory locations
- `movz`: fill out the remaining bytes of destination with zeros
- `movs`: fill out the remaining bytes by sign extension
  - replicating copies of most significant bit of the source operand
- `cltq` - sign-extend `%eax` to `%rax`
- memory references in x86-64 are _ALWAYS_ given with quad word (64 bytes) registers
- move/store value in `%rax` then `ret`
- function arguments stored in `%rdi` and `%rsi`
- local variables are _often_ kept in registers (rather than in memory locations)

### Pushing & Popping Stack Data

- !!!stack is in memory!!!
- `pushq` vs. `popq`
- in x86-64, stack grows towards _lower_ addresses
- `push` - `%rsp` _decreases_
- `pushq %rbp` is equivalent to:

```asm
subq $8, %rsp
movq %rbp, (%rsp)
```

- `popq %rax` is equivalent to:

```asm
movq (%rsp), %rax
addq $8, %rsp
```

## Arithmetic & Logical Operations

- **Load effective address**
  - `leaq`
  - a variant of `movq`
  - does _NOT_ reference memory at all
  - copies the effective address of source to destination
- Shift
  - `salb`
  - `salw`
  - `salq`

### Special Arithmetic Operations

- 16-byte - **oct word** by Intel
-
