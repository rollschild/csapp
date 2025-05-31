# Linking

- process of collecting & combining various pieces of code and data into a single file that can be **loaded** (copied) into memory and executed
- can be performed at:
  - compile time
  - load time
  - run time
- **linkers**
  - enable **separate compilation**

## Static Linking

- Linker performs two main tasks:

  1. **symbol resolution**
  2. **relocation**

- **symbol resolution**
  - to associate each symbol **reference** with exactly one symbol **definition**
  - symbols:
    - function
    - global variable
    - **static variable** (declared with `static`)

## Object Files

- Three forms
  - **relocatable object file**
  - **executable object file**
  - **shared object file**
- **object file formats**
  - Windows uses **Portable Executable (PE)** format
  - macOS uses **Mach-O**
  - Linux & modern Unix use **Executable and Linkable Format (ELF)**

## Relocatable Object Files (ELF)

- ELF header
- **section header table**
  - locations & sizes of various sections
- typical ELF contains following sections:
  - `.text` - machine code of compiled program
  - `.rodata` - read-only data
  - `.data` - _initialized_ global & `static` C variables
    - local variables are maintained at _runtime_ on stack; do _NOT_ appear in either `.data` or `.bss`
  - `.bss`
    - _uninitialized_ global and static C variables
    - with any global/`static` initialized to zero
    - occupies _NO_ space in the object file - just placeholder
  - `.symtab` - symbol table
    - about functions & global variables that are defined/referenced in the program
    - _NO_ entries for local variables
  - `.rel.txt` - list of locations in `.text` that will need to be modified when linker combines this object file with others
  - `.rel.data` - relocation info for any global variables that are referenced or defined by the module
  - `.debug` - debugging symbol table with entries for:
    - local variables and typedefs defined in the program
    - global variables defined & referenced
    - original C source file
    - _only_ present if compiled with `-g` option
  - `.line`
    - mapping between line numbers in original C program and machine code instructions in `.text`
    - _only_ present if compiled with `-g` option
  - `.strtab` - string table
    - for symbols tables in `.symtab` & `.debug` sections
    - for section names in section headers
    - a sequence of null-terminated character strings

## Symbols & Symbol Tables

- Three kinds:
  - **global symbols** defined by module and can be referenced by other modules
    - _nonstatic_ C functions & global variables
  - **global symbols** referenced by module but defined by some other module
    - **externals**
    - _nonstatic_ C functions & global variables defined in other modules
  - **local symbols** - defined/referenced _exclusively_ by module
    - static C functions
    - global variables defined with `static`
    - visible anywhere within module but _CANNOT_ be referenced by other modules
- local procedure variables defined with `static` are _NOT_ managed on stack
- symbol tables are built by **assemblers**
  - into the `.s` file
- Format of ELF entry in `.symtab` section:

  ```c
  typedef struct {
      int   name;      /* string table offset */
      char  type:4,    /* function or data (4 bits) */
            binding:4; /* local or global (4 bits) */
      char  reserved;  /* unused */
      short section;   /* section header index */
      long  value;     /* section offset or absolute address */
      long  size;      /* object size in bytes */
  } Elf64_Symbol;
  ```

- Three special pseudosections that do _NOT_ have entries in section header table:
  - **ABS** - symbols that should not be relocated
  - **UNDEF** - undefined symbols
  - **COMMON** - uninitialized data objects that are not yet allocated
- the GNU `readelf` program

## Symbol Resolution

- by associating each reference with exactly one symbol definition from the symbol tables of its input relocatable object files
- compiler ensures static local variables, which get local linker symbols, have unique names
- multiple object modules might define global symbols with the same name
- in C++ overloaded functions work because:
  - compiler encodes each unique method and parameter list combo into a unique name for the linker
  - **mangling** & **demangling**
  - class mangling:
    - number of characters in class's name +
    - class original name
  - method is encoded as:
    - original method name +
    - `__` +
    - mangled class name +
    - single letter encodings of each arg
  - `Foo::bar(int, long)` encoded as `bar__3Fooil`

### How Linkers Resolve Duplicate Symbol Names, on Linux Systems

- at compile time, compiler exports each global symbol to the assembler as either:
  - **strong**
  - **weak**
- assembler encodes this info implicitly in in the symbol table of the relocatable object file
- functions & initialized global variables get **strong** symbols
- uninitialized global variables get **weak** symbols
- Linkers used following rules:
  - multiple strong symbols with the same name are _NOT_ allowed
  - given a strong symbol and multiple weak symbols with the same name, choose the strong symbol
  - given multiple weak symbols with the same name, choose _any_ of the weak symbols
- try to invoke linker with flag such as GCC `-fno-common`
  - or `-Werror`

### Linking with Static Libraries

- **static library**
- on Linux, stored on disk in **archive** format
- `.a` suffix
- To create a static lib of some functions, use `ar`:

  ```console
  gcc -c lib1.c lib2.c
  ar rcs libsomething.a lib1.o lib2.o
  ```

- To use the lib,

  ```console
  gcc -c main.c
  gcc -static -o out main.o ./libsomething.a
  # gcc -static -o out main.o -L. -lsomething
  ```

### How Linkers Use Static Libraries to resolve references

- Linkers maintain three sets when going through files & archives in CLI from left to right:
  - `E` - relocatable object files
  - `U` - unresolved symbols
  - `D` - symbols that have been defined in previous input files
- the order of libs and object files on the command line _MATTERS_
- General rule:
  - place libraries at the _END_ of the command line

## Relocation

- **relocation**: linker merges input modules and assigns run-time addresses to each symbol
- at this point, linker knows the exact sizes of the code & data sections in its input object modules
- Two steps:
  1. **Relocating sections and symbol definitions**
  2. **Relocating symbol references within sections**

### Relocation Entries

- whenever assembler encounters a reference to an object whose ultimate location is unknown,
  - it generates a **relocation entry**
  - telling linker how to modify the reference when it merges the object file into an executable
  - placed in `.rel.text` and `.rel.data` for code and data, respectively
- ELF relocation entry

  ```c
  typedef struct {
      long offset;    /* offset of the reference to relocate */
      long type:32,   /* relocation type */
           symbol:32; /* symbol table index */
      logn addend;    /* constant part of relocation expression */
  } Elf64_Rela;
  ```

- ELF defines 32 relocation types:
  - `R_X86_64_PC32`
  - `R_X86_64_32` - relocate a reference that uses a 32-bit _absolute_ address
- x86-64 **small code model**
  - assumes total size of code + data in the executable file is smaller than 2 GB
  - thus can be accessed at run-time using 32-bit PC-relative addresses
  - default for GCC
  - programs larger than 2 GB can be compiled using:
    - `-mcmodel=medium` (medium code model)
    - `-mcmodel=large` (large code model)

### Relocating Symbol References

- `objdump -dx main.o`

## Executable Object Files

- **ELF header** includes **entry point**
- `.init` section defines a small function called `_init`, which
  - will be called by program's initialization code
- no `.rel` sections - executable is _fully linked_
- for any segment `s`, linker must choose a starting address `vaddr`, such that
  - `vaddr mod align = off mod align`
  - `off` is the offset of the segment's first section in the object file
  - `align` is the alignment specified in the program header

## Loading Executable Object Files

- **loader**
  - copies code & data in the executable object file from disk into memory - **loading**
  - run the program by jumping to its first instruction (**entry point**)
- Linux can invoke the loader by `execve`
- _Every_ linux program has a run-time memory image
  - code segment starts at address `0x400000`
  - followed by data segment
  - followed by run-time **heap**
    - grows _upwards_
  - largest legal user address: `2^48 - 1`
  - stack grows downwards
  - region above `2^48` - reserved for code/data in **kernel**
- Loader copies chunks of executable object file into the code & data segments
- Loader jumps to program's entry point
  - always the address of `_start` function
  - defined in the system object file `crt1.o` - same for all C programs
  - `_start` calls the **system startup function**, `__libc_start_main`
    - defined in `libc.so`
    - initializes the execution environment
    - calls the user-level `main` function
    - handles its return value
    - (if necessary, returns control to kernel)

## Dynamic Linking with Shared Libraries

- **shared library**
  - a.k.a. **shared object**
  - with `.so` suffix on linux
  - `.dll` on windows
  - object module that,
  - at _either_ run time or load time, can be loaded at an _arbitrary_ memory address and linked with a program in memory
  - **dynamic linking**
  - performed by **dynamic linker**
- a single copy of the `.text` section of a shared lib in memory can be shared by different running processes
- To build a shared lib:
  - `gcc -shared -fpic -o libshared.so libshared1.c libshared2.c`
  - `-fpic`: directs compiler to generate **position-independent code**
- To link a shared lib:
  - `gcc -o prog main.c ./libshared.so`
  - _none_ of the code/data sections from `libshared.so` are actually copied into the executable `prog`
  - instead, linker copies some relocation & symbol table info that will allow references to code/data in `libshared.so` to be resolved at load time
- when loader loads/runs executable `prog`,
  - it loads _partially linked_ `prog`
  - then checks the `.interp` section in `prog`
    - contains path name of the dynamic linker
    - `ld-linux.so`
  - passes control to dynamic linker
- Dynamic linker then:
  - relocates text/data of `libc.so` into some memory seg
  - relocates text/data of `libshared.so` into another memory
  - relocates any references in `prog` to symbols defined by `libc.so` and `libshared.so`
  - passes control to the app
    - locations of the shared libs are _fixed_ and do _NOT_ change during execution of the program

## Loading and Linking Shared Libs from Applications

- functions:
  - `void *dlopen(const char* filename, int flag);`
  - `void *dlsym(void *handle, char *symbol);`
  - `int dlclose(void *handle);`
  - `const char *dlerror(void);`
- To compile,
  - `gcc -rdynamic -o prog dll.c -ldl`

## Position-Independent Code (PIC)

- code that can be loaded without any relocations
- `-fpic`
- **Global Offset Table (GOT)**
  - created by compiler at the beginning of the data segment
  - contains 8-byte entry for each global data object (procedure _or_ global variable) referenced by object module
  - also generates relocation record for each entry in GOT
- **lazy binding**
  - defers the binding of each procedure address until the _first time_ the procedure is called
  - between GOT and **PLT** (procedure linkage table)
  - GOT part of data segment
  - PLT part of code segment
- PLT - an array of 16-byte code entries
  - `PLT[0]` special entry that jumps into the dynamic linker
  - each shared lib function called by the executable has its own PLT entry
  - each of these entries is responsible for invoking a specific function
  - `PLT[1]` - invokes system startup function `__libc_start_main`

## Library Interpositioning

- allows you to:
  - intercept calls to shared lib functions
  - and execute your own code _instead_
- users can:
  - trace number of times a lib func is called
  - can validate and trace its input and output
- Basic idea:
  - given a target func to be interposed on
  - create a wrapper function whose prototype is _identical_ to the target func
  - trick the system into calling the wrapper func instead
- **interpositioning** can occur:
  - compile time
  - link time
  - run time

### Compile Time Interpositioning

```c
// int.c
#include <malloc.h>

int main() {
    // ...
    int *p = malloc(32);
    free(p);
}

// malloc.h
#define malloc(size) mymalloc(size)
#define free(ptr) myfree(ptr)

void *mymalloc(size_t size);
void myfree(void *ptr);
```

- `gcc -DCOMPILETIME -c mymalloc.c`
- `gcc -I. -o intc int.c mymalloc.o`
- **interpositioning** happens because of `-I.` arg
  - tells C preprocessor to look for `malloc.h` in the current directory _before_ looking in the usual system dirs

### Link-Time Interpositioning

- Linux **static linker** supports link-time interpositioning with `--wrap f` flag
- `--wrap f` tells linker to
  - resolve references to symbol `f` as `__wrap_f`
  - resolve references to symbol `__real_f` as `f`

```c
// mymalloc.c
#ifdef LINKTIME
#include <stdio.h>

void *__real_malloc(size_t size);
void __real_free(void *ptr);

// malloc wrapper function
void *__wrap_malloc(size_t size) {
    void *ptr = __real_malloc(size); // call libc mallc
    printf("malloc(%d) = %p\n", (int)size, ptr);
    return ptr;
}

// free wrapper function
void __wrap_free(void *ptr) {
    __real_free(ptr); // call libc free
    printf("free(%p)\n", ptr);
}

#endif
```

- `gcc -DLINKTIME -c mymalloc.c`
- `gcc -c int.c`
- `gcc -Wl,--wrap,malloc -Wl,--wrap,free -o intl int.o mymalloc.o`
  - `-Wl,option` flag: passes `option` to linker
  - each comma is `option` is replaced with space
  - above passes following to linker:
    - `--wrap malloc`
    - `--wrap free`

### Run-Time Interpositioning

- requires access _only_ to the executable object file
- based on dynamic linker's `LD_PRELOAD` env variable
  - if it's set to list of share lib pathnames, separated by colons/spaces
  - dynamic linker (`ld-linux.so`) will search `LD_PRELOAD` libs _first_

```c
#ifdef RUNTIME
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

// malloc wrapper function
void *malloc(size_t size) {
    void *(*mallocp)(size_t size);
    char *error;

    mallocp = dlsym(RTLD_NEXT, "malloc"); // get address of libc malloc
    if((error = dlerror()) != NULL) {
        fputs(error, stderr);
        exit(1);
    }
    char *ptr = mallocp(size); // call libc malloc
    printf("malloc(%d) = %p\n", (int)size, ptr);
    return ptr;
}

// free wrapper function
void free(void *ptr) {
    void (*freep)(void *) = NULL;
    char *error;

    if(!ptr) return;

    freep = dlsym(RTLD_NEXT, "free"); // get address of libc free
    if((error = dlerror()) != NULL) {
        fputs(error, stderr);
        exit(1);
    }
    free(ptr); // call libc free
    printf("free(%p)\n", ptr);
}
#endif
```

- to build the shared lib that contains the wrapper functions:
  - `gcc -DRUNTIME -shared -fpic -o mymalloc.so mymalloc.c -ldl`
- compile the main program:
  - `gcc -o intr int.c`
- run the program in bash:
  - `LD_PRELOAD="./mymalloc.so" ./intr`

## Tools

- GNU `binutils`
- `ar`
- `strings`
- `strip`
- `nm`
- `size`
- `readelf`
- `objdump`
- `ldd` from Linux
