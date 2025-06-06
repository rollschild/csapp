# Optimizing Program Performance

- Some features of C, such as ability to perform pointer arithmetic and casting, make it _challenging_ for a compiler to optimize
- **instruction-level parallelism**
- **critical paths**

## Capabilities and Limitations of Optimizing Compilers

- GCC
  - `-Og` - basic optimizations
  - `-O1`, `-O2`, `-O3`
- **memory aliasing** - the case where two pointers may designate the same memory location
  - this is **optimization blocker**
- another **optimization blocker** is function calls
  - use **inline substitution**
  - GCC flag `-finline` or `-O1` and higher
  - blocks/disallows using symbolic debugger
- GCC _only_ attempts inlining for functions defined within a single file

## Expressing Program Performance

- **CPE (cycles per element)**
- nanoseconds & picoseconds
- **loop unrolling**
- cycles _per element_, not _per iteration_
- To compute **prefix sum**:

  ```c
  /*
  p0 = a0
  pi = p(i-1) + ai
  */
  void psum(float a[], float p[], long n) {
      long i;
      p[0] = a[0];
      for (i = 1; i < n; i++) {
          p[i] = p[i - 1] + a[i];
      }
  }
  void psum_loop_unroll(float a[], float p[], long n) {
      long i;
      p[0] = a[0];
      for (i = 1; i < n - 1; i+=2) {
          float mid_val = p[i - 1] + a[i];
          p[i] = mid_val;
          p[i+1] = mid_val + a[i+1];
      }

      // for even n (n = 2), finish remaining element
      if (i < n) {
          p[i] = p[i - 1] + a[i];
      }
  }
  ```

## Eliminating Loop Inefficiencies

- in `for` loop, the test condition must be evaluated on _every_ iteration
  - **code motion** - a general class of optimizations

## Reducing Procedure Calls

- procedure calls can incur overhead & also block most forms of program optimization

## Eliminating Unneeded Memory References

- pay attention to **aliasing**

## Understanding Modern Processors

- **instruction-level parallelism**
- **latency bound**
- **throughput bound**

### Overall Operation

- **superscalar** processors
  - can perform _multiple_ operations on _every_ clock cycle,
  - **out of order**: order in which instructions execute need _not_ correspond to their ordering in the machine-level program
- Overall design of a processor:
  - **ICU (instruction control unit)**
    - read a sequence of instructions _from memory_
    - generate (from these) a set of primitive ops to perform on program data
  - **EU (execution unit)**: execute these ops
- **out of order** processors better at achieving higher degrees of instruction-level parallelism
- ICU reads instructions from **instruction cache**
  - special high speed memory
  - contains most recently accessed instructions
- **branch prediction**
  - _guess_ whether a branch will be taken
  - _predict_ the target address for the branch
  - **misprediction** incurs _significant cost_ in perf
- **speculative execution**
  - process begins fetching & decoding instructions at where it predicts the branch will go
  - even _begins executing_ these ops _before_ it has been determined whether the branch prediction is correct or not
- **micro-operations**
  - decoded from instruction decoding logic
- on x86-64, instruction involving one/more memory references yields multiple ops
  - `addq %rax, 8(%rdx)`
  - separating memory references from arithmetic ops
- EU receives several ops on each clock cycle
  - ops dispatched to a set of **functional units**
    - specialized hardware
- **data cache**
  - high speed memory
  - contains most recently accessed data values
- **retirement unit**
  - keeps track of ongoing processing
  - makes sure it obeys sequential semantics of the machine-level program
  - controls updating of the registers
    - integer
    - floating point
    - SSE
    - AVX
  - _only_ updates program registers when instructions are being retired
    - happens only after processor certain that any branches leading to this instruction have been correctly predicted
- **register renaming**
  - most common mechanism for controlling comm of operands among the execution units
- the registers are updated _only after_ the processor is certain of the branch outcomes
- a typical floating-point adder contains 3 stages:
  - process the exponent values
  - add the fractions
  - round the result
- functional units with issue times of 1 cycle are **fully pipelined**
  - **issue time**: min number of clock cycles between two independent ops of the same type
  - can start a new operation every clock cycle
- Divider is _NOT_ **pipelined**

## Loop Unrolling

- Reduce number of iterations
  - by increasing number of elements computed on each iteration
- easily performed by compiler
- GCC will perform some forms of loop unrolling with `-O3` or higher

## Enhancing Parallelism

```c
// 2x2 loop unrolling
// by maintaining multiple accumulators, this approach makes better use
// of the multiple functional units and their pipelining capabilities
void combine(vec_ptr v, data_t *dest) {
    long i;
    long len = vec_length(v);
    long limit = len - 1;
    data_t *data = get_vec_start(v);
    data_t acc0 = IDENT;
    data_t acc1 = IDENT;

    // combine 2 elements at a time
    for (i = 0; i < limit; i += 2) {
        acc0 = acco0 OP data[i];
        acc1 = acco1 OP data[i+1];
    }

    // finish any remaining elements
    for (; i < len; i++) {
        acc0 = acc0 OP data[i];
    }

    *dest = acc0 OP acc1;
}
```

- **k x k loop unrolling**
- **reassociation transformation**
  - can reduce number of operations along the **critical path** in a computation
  - better performance by better utilizing the multiple functional units and their pipelining capabilities
  - _most_ compilers will _not_ attempt any reassociations of floating point ops
    - these ops not guaranteed to be associative
- **SSE**: streaming SIMD extensions
- **AVX**: advanced vector extensions
- SIMD
  - operates on entire vectors of data within single instructions
  - the vectors are held in special set of **vector registers**
    - `%ymm0` - `%ymm15`
  - current AVX vector registers are 32 bytes long
    - can hold 8 32-bit numbers or 4 64-bit numbers
    - either integer or floating-point
  - AVX instructions can perform vector ops on these registers, in parallel

## Limiting Factors

- Assume a program requires total of `N` computations of some operation, that the microprocessor has `C` functional units capable of performing the op and these units have an **issue time** of `I`
  - program will require _at least_ $N * I / C$ cycles to execute

### Register Spilling

- If a program has a degree of parallelism `P` that _exceeds_ number of available registers
  - compiler will resort to **spilling**
  - storing some temp values in memory, by allocating space on run-time stack
- modern x86-64 processors have 16 integer registers
  - can make use of the 16 YMM registers to store floating-point

### Branch Prediction and Misprediction Penalties

- **speculative execution**
- **conditional moves**
- Write code suitable with conditional moves
- branch prediction is only reliable for regular patterns
- performance: **conditional data transfer** >>> **conditional control transfer**
- GCC is able to generate conditional moves for code written in a more **functional** style
  - where, we use conditional ops to compute values, and
  - update the program state with these values
  - as opposed to a more **imperative** style, where we use conditionals to _selectively_ update program state
- A _more efficient_ (in terms of compiler code generation) merge sort:

  ```c
  void merge_sort(long src1[], long src2[], long dest[], long n) {
      long i1 = 0;
      long i2 = 0;
      long id = 0;

      while (i1 < n && i2 < n) {
          long v1 = src1[i1];
          long v2 = src2[i2];
          long take1 = v1 < v2;
          dest[id++] = take1 ? v1 : v2;
          i1 += take1;
          i2 += (1 - take1);
      }
  }
  ```

## Understanding Memory Performance

- modern processors have dedicated functional units to perform load/store ops
  - these units have internal buffers to hold sets of outstanding requests for memory ops

### Load Performance

- example of getting length of a linked list:

  ```c
  typedef struct ELE {
      struct ELE* next;
      long data;
  } list_ele, *list_ptr;

  long list_len(list_ptr ls) {
      long len = 0;
      while (ls) {
          len++;
          ls = ls->next;
      }
      return len;
  }
  ```

  ```asm
  ; inner loop of list_len
  ; ls in %rdi, len in %rax
  .L3:
    addq    $1, %rax     ; increment len
    movq    (%rdi), %rdi ; ls = ls->next
    testq   %rdi, %rdi   ; test ls
    jne     .L3
  ```

- the `movq` instruction is a _critical_ bottleneck

### Store Performance

- most cases the store op can operate in a fully pipelined mode
- **write/read dependency**
  - outcome of a memory read depends on a recent memory write
- store unit includes a **store buffer**
  - containing addresses & data of the store ops that:
    - have been issued to the store unit
    - but not yet been completed
    - completion involves updating data cache
- when a load op,
  - must check the entries in the store buffer for matching addresses
  - if a match found,
    - bytes being written to the address also being read
    - it retrieves the data entry as the result of the load op
- `movq %rax, (%rsi)` is translated into 2 ops:
  - `s_addr` computes address for the store op
    - creates entry in the store buffer
    - sets address field for that entry
  - `s_data` sets data field for the entry
  - two ops above are performed _independently_

## Performance Improvement Techniques

- Eliminate excessive function calls
- Move computations out of loops when possible
- Consider selective compromises of program modularity to gain greater efficiency
- Eliminate unnecessary memory references
  - introduce temp variables to hold intermediate results
  - store a result in an array or global variable _only when_ the final value has been computed
- _Unroll_ loops to reduce overhead and to enable further optimizations
- find ways to increase instruction-level parallelism by techniques such as:
  - multiple accumulators
  - reassociation
- Rewrite conditional ops in a functional style to enable compilation via **conditional data transfers**

## Identifying & Eliminating Performance Bottlenecks

- **program profiling**
- `gprof` - provided by unix
  - compile code in GCC with `-pg` flag
    - along with `-Og` to ensure compiler does not attempt to perform any optimizations via inline substitution
    - `gcc -Og -pg prog.c prog`
  - output: `gmon.out`
  - analyze the data in `gmon.out`: `gprof prog`
- `gprof`'s timing is _not_ very precise
  - for programs running for less than 1 mins - `gprof` provides only a rough estimate
-
