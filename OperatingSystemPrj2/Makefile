AVD_NAME=OsPrj-518030910188
KERNEL_ZIMG=./kernel/goldfish/arch/arm/boot/zImage
GOLDFISH = ./kernel/goldfish
TEST_DIR=./test
exe_name=test
TRACE_MODULE=./trace_module

HACKING_LIST=					\
arch/arm/include/asm/unistd.h   \
arch/arm/kernel/calls.S         \
arch/arm/kernel/sys_arm.c       \
include/linux/syscalls.h        \
include/linux/Yimin_oom.h       \
include/linux/Yimin_struct.h    \
mm/Kconfig                      \
mm/Makefile                     \
mm/Yimin_mm_limits.c            \
mm/Yimin_oom.c                  \
mm/oom_kill.c                   \
kernel/fork.c					\
kernel/exit.c

all: help

rsync:
	for file in $(HACKING_LIST); do \
		rsync -u hacking/$$file kernel/goldfish/$$file; \
	done

kernel: rsync
	make -j12 -C ${GOLDFISH}

menuconfig: rsync
	make menuconfig -C ${GOLDFISH}

help:
	@echo "To run the test:"
	@echo "    0. Run 'make kernel' to update hacking files and rebuild the kernel."
	@echo "    1. Run 'make emulator' to start the emulator."

emulator: 
	emulator -avd ${AVD_NAME} -kernel ${KERNEL_ZIMG} -no-window -show-kernel

handin:
	tar --exclude-vcs -cvf Prj2+518030910211.tar .

test-build: 
	@make -C ${TEST_DIR}

test: test-build
	cd ./test/obj/local/armeabi && adb push ./${exe_name} data/local
	@echo "move succeed"
	adb shell "cd data/local && chmod +x ./${exe_name} && su 10070 ./${exe_name} u0_a70 10000000  5000000  6000000  4000000  4000000"

module:
	make -C ${TRACE_MODULE}


.PHONY: test
