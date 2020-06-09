AVD_NAME=OsPrj-518030910188
KERNEL_ZIMG=./kernel/goldfish/arch/arm/boot/zImage
TEST_DIR=./test
exe_name=test

HACKING_LIST=					\
arch/arm/include/asm/unistd.h   \
arch/arm/kernel/calls.S         \
arch/arm/kernel/sys_arm.c       \
include/linux/syscalls.h        \
include/linux/Yimin_oom.h       \
include/linux/Yimin_struct.h    \
mm/Makefile                     \
mm/page_alloc.c                 \
mm/Yimin_mm_limits.c            \
mm/Yimin_oom.c                  \

all: help

rsync:
	for file in $(HACKING_LIST); do \
		rsync -u hacking/$$file kernel/goldfish/$$file; \
	done

kernel: rsync
	make -j12 -C kernel/goldfish

help:
	@echo "To run the test:"
	@echo "    0. Run 'make kernel' to update hacking files and rebuild the kernel."
	@echo "    1. Run 'make emulator' to start the emulator."
	@echo "    2. Run 'make testall' to run the test."

emulator: 
	emulator -avd ${AVD_NAME} -kernel ${KERNEL_ZIMG} -no-window -show-kernel

handin:
	tar --exclude-vcs -cvf Prj2+518030910211.tar .

test-build: 
	@make -C ${TEST_DIR}

test: test-build
	cd ./test/obj/local/armeabi && adb push ./${exe_name} data/local
	@echo "move succeed"
	adb shell "cd data/local && chmod +x ./${exe_name} && su 10070 ./${exe_name} u0_a70 1000000000 16000000 100000 200000 3000000 40000000 5000000"

.PHONY: test
