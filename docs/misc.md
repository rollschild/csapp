# Misc

## GCC flags

- `-std=c99` vs. `-std=c11`
- `-m64` vs. `-m32`
  - `-m32` runs on _either_ 64- or 32- bit machine
  - `-m64` _only_ runs on 64-bit machine
- `-Og`
  - apply a level of optimization that yields machine code that follows the overall structure of the original C code
  - `-O1` & `-O2`
    - higher level, more optimization, better performance
- `-S` - to see the assembly code
- `-c` - both compile and assemble the code
