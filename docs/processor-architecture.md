# Processor Architecture

- **Instruction Set Architecture (ISA)**
  - **sequential** instruction execution
  - each instruction fetched & executed to completion _before_ the next one begins

## Y86-64

- executes a complete instruction on every clock cycle
- **pipelined** processor
  - breaks execution of of each instruction into N steps
  - each step handled by a _separate_ section/stage of the hardware
- **hazard** conditions
- `hlt` - in x86-64
  - stops instruction execution
  - _NOT_ allowed to use by x86-64 programs!

### Instruction Encoding

- Instruction Set - encodings range between 1 - 10 bytes
  - 1-byte instruction specifier
  - (possibly) 1-byte register specifier
  - 8-byte constant word
- Initial byte - instruction type
  - split to two 4-bit parts
    - **code** part - high order
      - values ranging `0` - `0xB`
    - **function** - low order
      - _ONLY_ when a group of related instructions share a common code
      - e.g.:
        - `addq`
        - `subq`
        - `andq`
        - `xorq`
- Each register has a **Register Identifier**
  - `0` - `0xE`
- registers stored in file in CPU, **register file**
  - a _small_ **random access memory**
  - register IDs serve as addresses
- ID value `0xF`: _no_ register should be accessed
- **register specifier byte**
- the _additional_ 8-byte **constant word**
- all integers are **little-endian**
- How to encode `rmmovq %rsp, 0x123456789abcd(%rcx)`?
  1. `rmmovq` has initial byte `40`
  2. source register `%rsp` should be encoded in `rA` field of `rmmovq rA D(rB)`; `%rcx` in `rB` field
  3. `%rsp` has ID of `4`; `%rcx` has ID `1` -> register specifier byte of `42`
  4. displacement is encoded in 8-byte **constant word**
  5. pad with leading zeros to fill out 8 bytes -> `0x000123456789abcd`
  6. little endian -> reverse order: `cd ab 89 67 45 23 01 00`
  7. combine: `4042cdab896745230100`
- **CISC** vs. **RISC**
  - (early) **RISC**
    - has **fixed-length** encodings - _typically_ all instructions encoded as 4 bytes
    - arithmetic/logical ops _only_ use register operands
    - memory references _ONLY_ allowed by `load` & `store` instructions - **load/store architecture**
    - _NO_ condition codes
    - **register-intensive procedure linkage**
      - **registers** used for procedure args and return addresses
      - _typically_ has (many more) registers - up to 32

### Exceptions

- processor has a **status code**, `Stat`
- `Stat` potential values:
  - `AOK` - normal
  - `HLT`
  - `ADR` - invalid address
  - `INS` - invalid instruction
- there is a **max address** limit
  - access beyond this limit will trigger `ADR`
- when exception happens, processor invokes **exception handler**
  - can then invoke user-defined **signal handler**
- **assembler directives**
  - words starting with `.`
  - `.quad`
  - `.align 8` - align on 8-byte boundary
  - `.pos` - assembler should generate code starting at position `0`
- stack grows toward lower addresses
- `pushq` does two things:
  - decrements stack pointer by 8
  - writes a register value to memory
  - what about `pushq %rsp`? - convention:
    - push original value of `%rsp` on stack (used by x86-64), or:
    - push the _decremented_ value of `%rsp` on stack
- `popq %rsp` sets the stack pointer to the value read from memory
  - equivalent to `mrmovq (%rsp), %rsp`

## Logic Design and Hardware Control Language (HCL)

- logic value `1`: 1.0 volt
- logic value `0`: around 0.0 volt
- Three major components:
  - **combinational logic** - compute
  - **memory elements** - store
  - **clock signals** - control/regulate

### Logic Gates

- `&&`, `||`, `!`

### Combinational Circuits and HCL Boolean Expressions

- Restrictions:
  - every logic gate input must be connected to _exactly one_ of:
    - one of system inputs (**primary input**)
    - output connection of some memory element
    - output of some logic gate
  - outputs of _two or more_ logic gates _CANNOT_ be connected together
  - network must be **acyclic**
- HCL
  - `bool eq = (a && b) || (!a && !b)` - equals `1` when:
    - both `a` and `b` are `0`, _or_
    - both `a` and `b` are `1`
- **multiplexor**, a.k.a. **MUX**

### Word-Level Combinational Circuits and HCL Integer Expressions

- a word multiplexor's HCL:
  ```hcl
  word Out = [
      s: A;
      1: B; # default
  ];
  ```
- A general form of HCL **case expression**:
  ```hcl
  [
      select_1 : expr_1;
      select_2 : expr_2;
      .
      .
      .
      select_k : expr_k;
  ]
  ```
- **Arithmetic/Logical Unit (ALU)**
  - one important combinational circuit

### Set Membership

- general form `iexpr in { iexpr_1, iexpr_2, ..., iexpr_4 }`

### Memory and Clocking

- combinational circuits do _NOT_ store any information
- **sequential circuits** - systems that have state and perform on that state
- Two classes of memory devices:
  - **clocked registers**
    - clock signal controls the loading of the register with the value at its input
  - **Random Access Memories** - store multiple words
    - virtual memory
    - the **register file**
- **Hardware registers** serve as **barriers** between combinational logic in different parts of the circuit
  - values _only_ propagate from a register input to its output once every clock cycle at the _rising_ clock edge
-
