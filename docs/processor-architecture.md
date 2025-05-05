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

## Sequential Y86-64 Implementations

- Steps/stages of processing an instruction:
  - **Fetch**
    - read bytes of instruction from **memory**, using **PC** as the memory address
    - extract two 4-bit of **the instruction specifier code**:
      - **icode** - instruction code
      - **ifun** - instruction function
    - possibly fetches **register specifier byte**
    - possibly fetches an 8-byte constant `valC`
    - computes `valP` - address of instruction following current one in _sequential order_
      - `valP` === `PC` + length of the fetched instruction
  - **Decode**
    - reads _up to two_ operands from **register file**
    - some instructions read register `%rsp`
  - **Execute**
    - **ALU** performs operation specified by `ifun`
    - condition code possibly set
  - **Memory**
    - _may_ write data to memory
    - _may_ read data from memory
  - **Write back**
    - write _up to two_ results to register file
  - **PC update**
    - PC set to address of next instruction
- processor loops _indefinitely_
- x86-64 convention:
  - `push`: decrement the stack pointer _before_ writing
    - even though the actual updating of stack pointer does _not_ occur _until_ after memory operation has completed
  - `pop`: first read memory, _then_ increment stack pointer
- `call` & `ret`
  - similar to `pushq` & `popq`
  - but push & pop **program counter** values
  - `call` pushes `valP`, the address of instruction following `call`, to stack
  - `ret` pushes `valM`, the value popped from stack, to PC
- all processing by hardware units occurs within single clock cycle
- **PC** is the _only_ **clocked** register in SEQ

### SEQ Timing

- clocked registers:
  - PC
  - condition code register
- random access memory
  - register file
  - instruction memory
  - data memory
- program counter (PC) is loaded with a new instruction address _every_ clock cycle
- condition code register is loaded _only when_ an integer operation instruction is executed
- data memory written _only when_ `rmmovq`, `pushq`, or `call` is executed
- register ID `0xF` as port address -> no write should be performed at this port
- the clocking of registers & memories is all that's required to control the sequencing of activities
- **PRINCIPLE**: No reading back
  - processor _never_ needs to read back the state updated by an instruction in order to complete the processing of this instruction
- condition codes are _not_ set _until_ the clock rises to begin the next clock cycle
- Cycles of execution in SEQ
  - each cycle begins with the state elements set according to the _previous_ instruction
    - program counter
    - condition
    - condition code register
    - register file
    - data memory
  - signals propagate through the combinational logic, creating new values for the **state elements** (mentioned above)
    - these values are loaded to state elements to start the next cycle

### SEQ Stage Implementations

#### Fetch Stage

- includes the instruction memory hardware unit
- reads 10 bytes from memory at a time
  - using PC as address of the first byte (byte 0)
- First byte (PC byte) split into two 4-bit values
  - `icode`
  - `ifun`
  - `imem_error` - nop
- based on `icode`, compute 3 1-bit signals:
  - `instr_valid`: detect illegal instruction
  - `need_regids`: does this instruction include a register specifier type?
  - `need_valC`:does this instruction include a constant word
- `instr_valid` & `imem_error` - generate status code in memory stage
- Byte 1 split into:
  - register specifiers `rA` & `rB`, if `need_regids` is `1`
  - `rA` & `rB` both set to `0xF` (`RNONE`) if `need_regids` is `0`
- PC incrementer hardware unit
  - generates signal `valP`, based on:
    - current value of PC
    - the two signals `need_regids` & `need_valC`
  - for PC value `p`, `need_regids` value `r`, `need_valC` value `i`,
    - value generated by incrementer: `p + 1 + r + 8i`

#### Decode & Write-Back Stages

#### Memory Stage

- Task of _reading_/_writing_ data

#### PC Update Stage

```hcl
word new_pc = [
    # call. use instruction constant
    icode == ICALL : valC;
    # taken branch. use instruction constant
    icode == IJXX && Cnd : valC;
    # completion of RET instruction. use value from stack
    icode == IRET : valM;
    # default: use incremented PC
    1 : valP;
]
```

## General Principles of Pipelining

- increases **throughput** of system
- _may slightly_ increase latency
- **throughput**: number of customers served per unit time
- **latency**: time required to service an individual customer

### Computational Pipelines

- circuit delays measured in units of **picoseconds** (ps)
  - 10^-12 seconds
- throughput in giga-instructions per second (GIPS)

### Detailed Look at Pipeline Operation

- the transfer of instructions between pipeline stages is controlled by a clock signal
  - signal rising from 0 to 1 initiates the next set of pipeline stage evaluations

### Limitations

- **nonuniform partitioning**
- modern processors employ _very deep_ pipelines (15+ stages) attempting to max clock rate

### Pipelining a system with feedback

- **data dependency**

### Pipelined Y86-64 Implementations

- the PC update stage comes at the beginning of the clock cycle, _not_ at the end
- **SEQ+**
  - create state registers to hold signals computed during an instruction
  - then as a new clock cycle begins, the values propagate through the exact same logic to compute PC for the now-current instruction
  - `pIcode`?
- **circuit retiming**
  - changes state representation for a system without changing its logical behavior
-
