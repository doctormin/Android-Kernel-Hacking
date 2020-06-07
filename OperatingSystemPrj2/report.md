
## Adding Source File (.c) to Kernel
***Background***

When I am implementing global variable in the kernel, I add these  2 files:
```
goldfish
├── include
│   └── linux
│       └── Yimin_struct.h
└── mm
    └── Yimin_mm_limits.c

```
- `Yimin_struct.h` is used to define a struct which is the type of my global variable
- `Yimin_mm_limits.c` is used to initialize my global variable

But when I try to access my globle variable in my syscall defined in `goldfish/arch/arm/kernel/sys_arm.c` 
the error **Undefined Varible** comes out.

***Reason and Solution***

My c file (`Yimin_mm_limits.c`) is not linked with other files of the OS in after compliation stage. So even though I declare `extern struct Yimin_struct Yimin_mm_limits` in the syscall funtion. The variable can still not be found.

To solve this, all I need to do is adding the following line to the `Makefile` in `goldfish/mm` where my c file locates.
```
obj-y += Yimin_mm_limits.o
```




## About `EXPORT_SYMBOL()` and Symbol Table

### Symbol Table
***definition***

 - Kernel symbol table is nothing but a look-up table between symbol names and their addresses in memory. 
 - When a module is loaded into Kernel memory using insmod or modprobe utility, any symbol exported by the module becomes part of the Kernel symbol table. 
  
***More Information***
- This kernel symbol table is loaded into memory as part of the kernel boot process
  

### `EXPORT_SYMBOL()` & `EXPORT_SYMBOL_GPL()`

- `EXPORT_SYMBOL()`
  - Exports a given symbol to all loadable modules

- `EXPORT_SYMBOL_GPL()`
  - Exports a given symbol to only those modules that have a *GPL-compatible license*



