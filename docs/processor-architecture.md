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
-
