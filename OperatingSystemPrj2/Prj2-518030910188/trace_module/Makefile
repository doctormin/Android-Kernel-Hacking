obj-m := trace_module.o
KID := ../kernel/goldfish
CROSS_COMPILE=arm-linux-androideabi-
CC=$(CROSS_COMPILE)gcc
LD=$(CROSS_COMPILE)ld

all: clean 
	make -C $(KID) ARCH=arm CROSS_COMPILE=$(CROSS_COMPILE) M=$(shell pwd) modules
	adb push ./trace_module.ko data/misc
	adb shell "cd data/misc && insmod trace_module.ko"

clean: rmmod
	rm -rf *.ko *.o *.mod.c *.order  *.symvers  .*.cmd .tmp*

rmmod:
	adb shell "cd data/misc && rmmod trace_module.ko && rm trace_module.ko"