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
