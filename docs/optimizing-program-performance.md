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
