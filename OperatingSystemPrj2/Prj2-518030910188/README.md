```
.
├── hacking
│   ├── arch
│   │   └── arm
│   │       ├── include
│   │       │   └── asm
│   │       │       └── unistd.h
│   │       └── kernel
│   │           ├── calls.S
│   │           └── sys_arm.c
│   ├── include
│   │   └── linux
│   │       ├── syscalls.h
│   │       ├── Yimin_oom.h
│   │       └── Yimin_struct.h
│   ├── kernel
│   │   ├── exit.c
│   │   └── fork.c
│   └── mm
│       ├── Kconfig
│       ├── Makefile
│       ├── oom_kill.c
│       ├── Yimin_mm_limits.c
│       └── Yimin_oom.c
├── Makefile
├── README.md
├── report-518030910188.pdf
├── test
│   ├── jni
│   │   ├── Android.mk
│   │   └── test.c
│   └── Makefile
├── test-result.png
└── trace_module
    ├── Makefile
    └── trace_module.c

13 directories, 22 files
```

- hacking: contains all the files I modified or added to the kernel (can be sychronized with the kernel using my Makefile on one click!)
- test:    please place the test programme into test/jni
- trace_module: contains a module named `trace_module`, this module acts as a notifier and detecter(plz refer to my report)
- Makefile:     
  1. Run 'make menuconfig' to choose Yimin's oom-killer in the menuconfig GUI
    2. Run 'make kernel' to update hacking files(rsync) and rebuild the kernel.
    3. Run 'make emulator' to start the emulator.
    4. Run 'make module' to compile and install the module
    5. Run 'make test' to build and push and run the test program in ./test/jni
- report: project report
- test-result.png: a simple demo (I want to show details in presentation)