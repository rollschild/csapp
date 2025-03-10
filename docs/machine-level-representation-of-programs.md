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
