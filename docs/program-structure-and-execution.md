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
- For two's complement, `~x + 1 = -x`
- Bit pattern of form `[0,...0,1,...1]` with `w - k` zeros followed by `k` ones
  - can be represented as `(1 << k) - 1`

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

### Conversions between Signed and Unsigned

- From Tow's complement to unsigned:

  - $T2U_w(x) = x + 2^w$ if `x < 0`
  - $T2U_w(x) = x$ if `x >= 0`

- From Unsigned to Two's complement:

  - $U2T_w(u) = u$ if `u <= TMax_w`
  - $U2T_w(u) = u - 2^w$ if `u > TMax_w`

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

## Integer Arithmetics

### Two's-Complement Addition

```markdown
$x + y = x + y - 2^{w}$ if $2^{w-1} <= x + y$ - positive overflow
$x + y = x + y$ if $-2^{w-1} <= x + y < 2^{w-1}$
$x + y = x + y + 2^{w}$ if $x + y < -2^{w-1}$ - negative overflow
```

- for `x` and `y` in the range `TMin_w <= x, y <= TMax_w`, and `s = x + y`
  - `s` has positive overflow if and only if `x > 0` and `y > 0` but `s <= 0`
  - `s` has negative overflow if and only if `x < 0` and `y < 0` but `s >= 0`

### Two's-Complement Negation

- `TMin`'s additive inverse is itself
  - $TMin_w + TMin_w = -2^{w-1} - 2^{w-1} = -2^w$, which causes negative overflow
  - hence $TMin_w + TMin_w = -2^w + 2^w = 0$
- `~x = -x - 1`

### Truncation

- Unsigned:
  - `x' = x mod 2^k`, where `x'` and `x` are signed equivalent of bit vectors of result and original
- Two's complement
  - `x' = U2T_k(x mod 2^k)`, where `x'` and `x` are signed equivalent of bit vectors of result and original

### Unsigned Addition

```markdown
$x + y = x + y$ if $x + y < 2^{w}$
$x + y = x + y - 2^{w}$ if $2^{w} <= x + y < 2^{w+1}$
```

- for `x` and `y` in the range `0 <= x, y <= UMax_w`, and `s = x + y`
  - `s` has overflow if and only if `s < x` (or `s < y`)

### Unsigned Multiplication

- Truncating an unsigned number to `w` bits is === computing its value modulo $2^w$
- $x * y = (x⋅y) mod 2^w$

### Two's-Complement Multiplication

- in C, signed multiplication performed by truncating the $2w$-bit product to `w` bits
  - first, compute its value modulo $2w$
  - then, convert from unsigned to two's complement
- $x * y = U2T_w((x⋅y) mod 2^w)$
  - $U2T_w$: unsigned to two's complement

```c
/**
Determine whether two signed arguments can be multiplied without causing overflow
*/
int tmult_ok(int x, int y) {
    int p = x * y;
    // Either x is zero, or dividing p by x gives y
    return !x || p/x == y;
}
```

### Multiplying by Constants

- Historically, multiplication slower than addition/subtraction/shifting
- `x * 14`:
  - `(x << 3) + (x << 2) + (x << 1)`
  - `(x << 4) - (x << 1)`

### Dividing by Powers of 2

- Integer division is _even slower_ than integer multiplication
  - 30+ clock cycles!
- integer division always round toward zero
- Two's complement division
  - `(x + (1 << k) - 1) >> k` yields `x / 2^k`
  - adding a **bias**
- `x / 2^k` === `(x < 0 ? x + (1<<k) - 1 : x) >> k`

## Floating Point

- **IEEE Floating-Point Representation**
  - $V = (-1)^S × M × 2^E$
  - `M`: **significand**

### Normalized Value

- most common case
- `E`
  - when `exp` is neither all `0`s nor all `1`s
  - the exponent field interpreted as a _signed_ integer in **biased** form
  - exponent value, `E`: `E = e - Bias`
  - `e` has bit representation $e_{k-1},e_{k-2}...e_1,e_0$
    - here `e` is the actual bits in the bit representation of the value
  - `Bias` == $2^{k-1} - 1$
- `frac`, `0 <= f < 1`
  - binary representation $0.f_{n-1}...f_1f_0$
  - the actual value it represents: $f / 2^n$, where `n` is number of f bits
- **significand**: $M = 1 + f$
  - **implied leading 1**
  - bit representation: $1.f_{n-1}...f_1f_0$
  - `1 <= M < 2`

### Denormalized Value

- when exponent all zeros
- $E = 1 - Bias$
- $M = f$
- Two purposes:
  - represent numeric `0` - sign bit is `0`, exponent field all `0`s, fraction all `0`s
    - `-0.0` when sign bit is `1` but all others are `0`s
  - numbers _very_ close to `0.0`
    - **gradual underflow**

### Special Values

- when exponent all `1`s
- when fraction field all `0`s:
  - `+∞` when sign bit `0`
  - `-∞` when sign bit `1`
- when fraction _nonzero_: `NaN`

### Rounding

- 4 different rounding modes
  - round to even (round to closest)
  - round toward zero
  - round down
  - round up

### Floating-Point Operations

- $+∞ - ∞ = NaN$
- With single-precision,
  - $1e20 × (1e20 - 1e20) = 0$
  - $1e20 × 1e20 - 1e20 × 1e20 = NaN$

### Floating Point in C

- use the **round to even (closest)** mode, on machines supporting IEEE
- In GCC, if:
  ```c
  #define _GNU_SOURCE 1
  #include <math.h>
  ```
  - then `INFINITY` is `+∞`
  - `NAN` is $NaN$
- when casting:
  - `double` to `float`: can overflow to `+∞` or `-∞`
  - `double`/`float` to `int`: round to zero; may overflow
    - On Intel-compatible arch, bit pattern `[1....0]` ($TMin_w$ for word size `w`)
    - as **integer indefinite**
    - `(int)+1e10` yields `-21483648`
