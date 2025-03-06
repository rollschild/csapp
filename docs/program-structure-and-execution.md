# Program Structure and Execution

## Interpretations

- **unsigned**
- **two's complement**
- **floating-point**

## Info Storage

- **virtual address space**
  - a conceptual image presented to the machine-level program
  - consists of:
    - DRAM
    - flash memory
    - disk storage
    - special hardware
    - OS software
- `float` - single precision
- `double` - double precision
- `sizeof(T)` - number of bytes required to store object of type `T`

```c
void inplace_swap(int* x, int* y) {
    *y = *x ^ *y;
    *x = *x ^ *y;
    *y = *x ^ *y;
}
```

### Logical Operations

- treat _any_ nonzero argument as representing `true` and argument `0` as `false`

### Shift Operations in C

- right shift
  - **logical** - fills the left end with `k` zeros
  - **arithmetic** - fills the left end with `k` repetitions of the most significant bit
    - used by almost _all_ compiler/machine combos for signed data

## Integer Representations

- format macros
  - `PRId32` - format conversion specifier to output a signed decimal integer value of type `std::int32_t`
  - `PRIu64` - format conversion specifier to output an _unsigned_ decimal integer value of type `std::uint64_t`

```c
// these two are equivalent
printf("x = %" PRId32 ", y = %" PRIu64 "\n", x, y);
printf("x = %d, y = %lu\n", x, y);
```

- `<limits.h>`

  - `INT_MAX`
  - `INT_MIN`
  - `UINT_MAX`

- when operation between an unsigned and a signed, C implicitly casts the signed argument to _unsigned_

  - then assumes both operands are _nonnegative_

  ```c
  /* prototype */
  size_t strlen(const char* s);

  /* BUGGY!!! */
  /**
  This uses unsigned arithmetic;
  when s is shorter than t, strlen(s) - strlen(t)
  should be negative, but instead it will result in a large, nonnegative number;
  To fix this: `return strlen(s) > strlen(t);`
  */
  int strlonger(char *s, char*t) {
      return strlen(s) - strlen(t) > 0;
  }
  ```

  - one way to avoid this ^^^, _NEVER_ use unsigned numbers

- in `<limits.h>`,

  ```c
  #define INT_MAX 2147483647
  #define INT_MIN (-INT_MAX - 1)
  ```

- relative order from one data size to another:
  - first, change the size
  - then, change the type

## GCC flags

- `-std=c99` vs. `-std=c11`
- `-m64` vs. `-m32`
  - `-m32` runs on _either_ 64- or 32- bit machine
  - `-m64` _only_ runs on 64-bit machine
