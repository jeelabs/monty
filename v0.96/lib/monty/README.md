This is the Monty virtual machine implementation. **Only** the `monty.h` header file  
is for public use in other libraries. The rest is only to be included from `main()`.

Files in this area are processed by the `src/codegen.py` code generator, which  
will insert and replace lines in the source code at all places marked by `//CG`.  
This process is automatic when using `make run` in the top level directory.
