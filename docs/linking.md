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
-
