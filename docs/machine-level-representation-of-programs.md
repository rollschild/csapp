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

### `switch` statements

- an efficient implementation using **jump table**
- **jump table**
  - an array
  - where entry `i` is the address of a code segment implementing the action the program should take when switch index equals `i`
  - used when there are a number of cases spanning a _small_ range of values

```c
void switch_eg(long x, long n, long *dest) {
    long val = x;
    switch(n) {
      case 100:
        val *= 13;
        break;
      case 102:
        val += 10;
        /* fall through */
      case 103:
        val += 11;
        break;
      case 104:
      case 106:
        val *= val;
        break;
      default:
        val = 0;
    }
    *dest = val;
}

void switch_eg_impl(long x, long n, long *dest) {
    // table of code pointers
    // each entry is address of a block of code
    static void *jt[7] = {
        &&loc_A, &&loc_def, &&loc_B, &&loc_C, &&loc_D, &&loc_def, &&loc_D
    };
    unsigned long index = n - 100;
    long val;

    if (index > 6)
        goto loc_def;
    /* multiway branch */
    goto *jt[index]; // GCC extension

loc_A: /* case 100 */
    val = x * 13;
    goto done;
loc_B: /* case 102 */
    x = x + 10;
    /* fall through */
loc_C: /* case 103 */
    val = x + 11;
    goto done;
loc_D: /* case 104, 106 */
    val = x * x;
    goto done;
loc_def: /* default case */
    val = 0;
done:
    *dest = val;
}
```

- the assembly version:

```assembly
; x in %rdi, n in %rsi, dest in %rdx
switch_eg:
  subq    $100,          %rsi    ; index = n - 100
  cmpq    $6,            %rsi    ; compare index:6
  ja      .L8                    ; if >, goto loc_def
  jmp     *.L4(,%rsi,8)          ; goto *jg[index] - indirect jump
.L3:                             ; loc_A
  leaq    (%rdi,%rdi,2), %rax    ; 3 * x
  leaq    (%rdi,%rax,4), %rdi    ; val = x + 3x * 4 = 13 * x
  jmp     .L2                    ; goto done
.L5:                             ; loc_B
  addq    $10,           %rdi    ; x = x + 10
.L6:
  addq    $11,           %rdi    ; val = x + 11
  jmp     .L2
.L7:
  imulq   %rdi,          %rdi    ; val = x * x
  jmp     .L2
.L8:                             ; loc_def
  movl    $0,            %edi    ; val = 0
.L2:
  movq    %rdi,          (%rdx)  ; *dest = val
  ret
```

- the jump table:

  ```assembly
    .section    .rodata
    .align      8       ; align address to multiple of 8
  .L4:
    .quad       .L3     ; case 100: loc_A
    .quad       .L8     ; case 101: loc_def
    .quad       .L5     ; case 102: loc_B
    .quad       .L6     ; case 103: loc_C
    .quad       .L7     ; case 104: loc_D
    .quad       .L8     ; case 105: loc_def
    .quad       .L7     ; csae 106: loc_D
  ```

- GCC jump tables - extension to C
- `&&`: pointer to a code location
- `jmp *.L4(,%rsi,8)` - indirect jump
  - operand specifies memory location indexed by register %eax

## Procedures

- **passing control**
- **passing data**
- **allocating & deallocating memory**

### Run-Time Stack

- x86-64 stack grows towards _lower_ addresses
- `pushq` & `popq`
- when an x86-64 procedure requires storage beyond that it can hold in registers,
  - it allocates space on stack
- stack frames for _most_ procedures are _fixed size_
- if/when procedure has 6 or fewer args, all parameters can be passed in registers
- many functions do _not_ even require a stack frame, when/if:
  - all local variables can be held in registers, and
  - function does _not_ call any other functions
  - a.k.a. **leaf procedure**

### Control Transfer

- P calling Q
- set program counter (`PC`) to starting address of `Q` code
- `call Q`
  - push address `A` onto stack
  - set PC to the beginning of `Q`
  - `A` - **return address**
    - the address of the instruction _immediately following_ the `call`
- `ret`
  - pops an address `A` off the stack
  - sets PC to `A`
- `call` can be **direct** or **indirect**
  - `call *Operand` - indirect

### Data Transfer

- with x86-64, most data passing to/from takes place via **registers**
  - `%rdi`, `%rsi`
- values returned in `%rax`
- up to _six_ integral (integer & pointer) arguments can be passed via registers
  - in the following order:
    - `%rdi`, `%rsi`, `%rdx`, `%rcx`, `%r8`, `%r9`
    - `%edi`, `%esi`, `%edx`, `%ecx`, `%r8d`, `%r9d`
    - `%di`, `%si`, `%dx`, `%cx`, `%r8w`, `%r9w`
    - `%dil`, `%sil`, `%dl`, `%cl`, `%r8b`, `%r9b`
- if arguments more than six, others passed on stack
  - caller must allocate stack frame for args 7 to `n`
  - arg 7 on top of stack
  - if passed onto stack, all data sizes rounded up to be multiples of `8`

### Local Storage on Stack

- local data must be stored in memory when/if:
  - not enough registers to hold all local data
  - address operator `&` applied to local variable - we must generate an address for it
  - some local variables are arrays/structures
- space allocated on stack by decrementing stack pointer `%rsp`

### Local Storage in Registers

- **callee-saved** registers:
  - `%rbx`, `%rbp`
  - `%r12` - `%r15`
  - the callee must _preserve_ the values of these registers
- To preserve a register value:
  - not change it at all, or
  - push the original value on stack, alter it, then pop the old value from stack before returning
    - creates a portion of stack frame labeled **saved registers**
    - e.g. `pushq %rbx` & `pushq %rbp`
- all other registers (except `%rsp`) are **caller-saved** registers
  - can be modified by any function

### Recursive Procedures

- each procedure call has its own private space on stack

```c
long rfact(long n) {
    long result;
    if (n <= 1) {
        result = 1;
    } else {
        result = n * rfact(n - 1);
    }
    return result;
}
```

```assembly
; long rfact(long n)
; n in %rdi
rfact:
  pushq    %rbx           ; save %rbx
  movq     %rdi,     %rbx ; store n in callee-saved register
  movl     $1,       %eax ; set return value = 1
  cmpq     $1,       %rdi ; compare n:1
  jle      .L35           ; if <=, goto done
  leaq     -1(%rdi), %rdi ; compute n - 1
  call     rfact          ; call rfact(n - 1)
  imulq    %rbx,     %rax ; multiply result by n
.L35:                     ; done
  popq     %rbx           ; restore %rbx
  ret
```

## Array Allocation & Access

- `T A[N]`
  - allocates `L * N` bytes of _contiguous_ memory, where `L` is size of data type `T`
  - `A` is pointer to beginning of the array
- `movl (%rdx,%rcx,4), %eax`
  - to evaluate `E[i]`, where `E` is array of `int`s
  - `E` stored in `%rdx`
  - `i` in `%rcx`
- `A[i]` === `*(A + i)`

### Nested Arrays

- implemented in **row-major** order
- `T D[R][C]`
  - `&D[i][j] = x_d + L(C * i + j)`
  - `x_d` is starting location of `D`

### Fixed-Size Arrays

```c
// GOOD coding practice!!!
#define N 16
typedef int fix_matrix[N][N]
```

### Variable-Size Arrays

- _Historically_, C only supported multidimensional arrays where sizes (possible exception of the first dimension) could be determined compile time
- C99 introduced capability of having array dimension expressions that are _computed_ as array being allocated

## Heterogeneous Data Structures

### Structures

- all components of a structure are stored in contiguous region of memory
- pointer to structure is the address of its first _byte_

```c
struct rec {
    int i;
    int j;
    int a[2];
    int* p;
};
```

- for an `r` that's pointer to `struct rec`, `r->p = &r->a[r->i + r->j];`:

  ```assembly
  ; r in %rdi
  movl    4(%rdi),         %eax     ; r->j
  addl    (%rdi),          %eax     ; r->j + r->i
  cltq                              ; extend to 8 bytes
  leaq    8(%rdi,%rax,4), %rax      ; &r->a[r->i + r->j]
  movq    %rax,            16(%rdi) ; store in r->p
  ```

### Unions

- allowing single object to be referenced according to multiple types
- used when _we know in advance that the use of two different fields in a data structure will be mutually exclusive_

```c
// common method to define a tree's internal nodes and leaf nodes
typedef enum {
    N_LEAF,
    N_INTERNAL
} nodetype_t;

// this structure requires 24 bytes
struct node_t {
    nodetype_t type; // 4 bytes
    // additional 4 bytes needed here,
    // between type and union
    union {
        struct {
            struct node_t* left;
            struct node_t* right;
        } internal;
        double data[2];
    } info;
};
```

- cast `double` to `unsigned long`

  ```c
  unsigned long double2bits(double d) {
      union {
          double d;
          unsigned long u;
      } temp;
      temp.d = d;
      return temp.u;
  }
  ```

- Pay attention to byte-ordering when combining data types of different sizes!

  ```c
  double uu2double(unsigned word0, unsigned word1) {
      union {
          double d;
          unsigned u[2];
      } temp;

      // on little endian, word0 will become low-order 4 bytes of d
      // on big endian, word0 will become high-order 4 bytes of d
      temp.u[0] = word0;
      temp.u[1] = word1;

      return temp.d;
  }
  ```

### Data Alignment

- Intel recommends data be aligned to improve memory system performance
- **Any primitive object of K bytes must have an address that is a multiple of K**
- `.align 8`
  - ensures data following it will start with an address that is a multiple of `8`
- gaps/bytes inserted by compiler?
- For structures,
  - any pointer `p` of type `struct S*` must satisfy k-byte alignment

## Combining Control and Data in Machine-Level Programs

- the machine often uses `leaq` to take address, ie. `&` operator
- **Casting from one type of pointer to another changes its type but _NOT_ its value**
  - casting has _higher_ precedence than addition

### GDB

- `gdb <prog>`

### Buffer Ovlerflow

- **stack randomization**
  - `alloca`
  - one of a _larger_ class of techniques, **address-space layout randomization (ASLR)**
- **ASLR** - different parts of the program are loaded into _different_ regions of memory each time program is run, including:
  - program code
  - library code
  - stack
  - global variables
  - heap data
- **stack protector** - recent GCC additions
  - store a special **canary** value in stack frame
  - between any local buffer and rest of stack state
  - a.k.a. **guard value**
  - generated randomly each time program runs
  - `-fno-stack-protector` to prevent GCC from inserting this code
  - if corrupted, `call __stack_chk_fail`
  - _only inserted when_ there is a local buffer of type `char` in the function
- Limit execution code regions
  - limit which memory regions hold executable code
  - In typical programs, only the portion of memory holding the code generated by compiler need to executable
    - other regions restricted to reading/writing
  - historically, x86 merged the read & execute access controls into a single 1-bit flag
  - recently AMD introduced an **NX** (no-execute) bit into memory protection
    - separates read & execute access
    - stack can be marked as readable & writable, but _NOT_ executable
    - checking whether a (memory) page is executable is performed in hardware - no penalty in efficiency
- some types of programs require ability to dynamically generate/execute code
  - JIT compilers

### Variable-Size Stack Frames

- when functions call `alloca`
  - allocates arbitrary number of bytes of storage on stack
- also when code declares a local array of _variable_ size
- if a local variable's address is computed, then this variable must be on stack..?
- `%rbp` - frame pointer (base pointer)
  - code must save the previous version of `%rbp` on stack,
    - since it's **callee-saved**
  - code references fixed-length local variables as offsets relative to `%rbp`
- `leave` instruction: restore `%rbp` and `%rsp` to their previous values; equivalent to:

  ```assembly
  ; stack pointer first set to position of saved value %rbp
  ; then this value popped from stack into %rbp
  ; effetively deallocating the entire stack frame
  movq %rbp, %rsp
  popq %rbp
  ```

- nowadays frame pointer _only_ used in code where stack frame may be of _variable_ size
- `(x + 7) >> 3` - round `x` up to the nearest multiple of `8`
  - see [Dividing by powers of 2](program-structure-and-execution.md#dividing-by-powers-of-2)
