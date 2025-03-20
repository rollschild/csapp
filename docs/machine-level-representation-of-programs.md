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
  - e.g. when `x` stored in `%rdi`, `leaq 7(%rdi), %rax` means `temp = x + 7`
- Shift
  - `salb`
  - `salw`
  - `salq`

### Special Arithmetic Operations

- 16-byte - **oct word** by Intel
- `<inttypes.h>`
  - does _not_ provision 128-bit values
- GCC has support for 128-bit integers, `__int128`
- `imulq`/`mulq`
  - _could_ result in 128-bit value
  - one arg must be in `%rax`
  - the other arg is the operand
  - product stored in:
    - `%rdx` for the high-order 64 bits
    - `%rax` for the low-order 64 bits
- `idivq`/`divq`
  - `idivq S`/`divq S`
  - `R[%rdx]:R[%rax] mod S` in `R[%rdx]`
  - `R[%rdx]:R[%rax] ÷ S` in `R[%rax]`
- In most applications of 64-bit addition, dividend is given as 64-bit,
  - should be stored in `%rax`
  - bits of `%rdx` should be set to:
    - all zeros (unsigned arithmetic), or
    - the sign bit of `%rax` - can be performed by `cqto`
- `cqto` - convert to oct word
  - reads _NO_ operands
  - operates on `%rax`
  - reads sign bit of `%rax` and copies to all `%rdx`

## Control

- CPU maintains a set of _single-bit_ **condition code** registers
  - to describe attributes of _most recent_ arithmetic/logical operation
  - `CF` - carry flag
    - carry out of most significant bit
    - detect overflow for _unsigned_ operations
  - `ZF` - zero flag
    - most recent operation yielded zero
  - `SF` - sign flag
    - most recent operation yielded a negative value
  - `OF` - overflow flag
    - most recent operation caused a two's-complement overflow
    - negative or positive
- logical ops such as `XOR`
  - carry & overflow flags set to zero
- `INC` & `DEC` set overflow & zero flags, but carry flag unchanged
- Two classes set condition codes without updating any other registers
  - `CMP`
    - `cmpb`
    - `cmpw`
    - `cmpl`
    - `cmpq`
  - `TEST`
    - `testb`
    - `testw`
    - `testl`
    - `testq`

### Accessing condition codes

- compare two `long`:

  ```assembly
  ; int comp(data_t a, data_t, b)
  ; a in %rdi, b in %rsi
  comp:
    cmpq    %rsi, %rdi ; compare a:b
    setl    %al        ; set low-order byte of %eax to 0 or 1
    movzbl  %al, %eax  ; clear rest of %eax (and rest of %rax)
    ret
  ```

- always the form `t = a - b`
- **exclusive OR** between `SF` and `OF` indicates whether `a < b`/`a > b` while overflow occurs
  `SF ^ OF`

### jump

```assembly
movq $0, %rax
movq (%rax), %rdx ; dereference a NULL pointer
```

- when generating the object-code file, the assembler determines the addresses of the destination of all labeled instructions
  - and encodes the **jump targets**
- **indirect jump**: jump target is read from a register or memory location
  - using `*`
  - `jmp *%rax`
  - `jmp *(%rax)` - reads value from memory (address in `%rax`) as jump target
- encodings
  - assembler, then later the linker, generate proper encodings of the jump targets
  - **PC relative**
    - encode difference between:
      - address of target instruction and address of the instructions
      - address of instruction _immediately following_ the jump
    - encoded in 1, 2, or 4 bytes
- PC (program counter) points to the instruction _right after_ the `jmp`, _NOT_ the `jmp` itself
- `rep; ret` - special thing on AMD to properly handle jumping falling through to `ret`
- **conditional data transfer** vs. **conditional control transfer**
  - former can _outperform_ latter
- processors achieve high performance through **pipelining**
  - an instruction si processed via a sequence of stages
- **branch prediction logic**
  - wrong guess could incur penalty of 15-30 cycles
  - for some highly random/unpredictable operations like `test x < y`, branch misprediction _dominates_ performance
- conditional move vs. conditional jump
- **conditional moves**
  - `cmov S, R`, etc
  - source & dest can be 16, 32, or 64 bits long
  - does _NOT_ always improve efficiency
  - In GCC, only use conditional moves when the two expressions can be computed very easily

### Loops

#### while loops

- multiple ways to translate `while` loop to machine code
  - two of which used in GCC
    - **jump to middle**
      - with option `-Og`
    - **guarded do**
      - with option `-O1`

#### `for` loops

```c
long reverse_bits(unsigned long x) {
    long val = 0;
    long i;

    for (i = 64; i != 0; i--) {
      val = (val << 1) | (x & 0x1);
      x >>= 1;
    }

    return val;
}
```

```assembly
; x in %rdi
; uses guarded do
; compiler skips initial test because i is initially 64 and > 0
reverse_bits:
  movl    $64,  %edx
  movl    $0,   %eax
.L10:
  movq    %rdi, %rcx
  andl    $1,   %ecx
  addq    $rax, %rax
  orq     %rcx, %rax
  shrq    %rdi       ; shift right by 1
  subq    $1,   %rdx
  jne     .L10
  rep; ret
```

-
