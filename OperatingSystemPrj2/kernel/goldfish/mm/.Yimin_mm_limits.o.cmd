cmd_mm/Yimin_mm_limits.o := arm-linux-androideabi-gcc -Wp,-MD,mm/.Yimin_mm_limits.o.d  -nostdinc -isystem /home/minozhao/android-ndk-linux/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64/bin/../lib/gcc/arm-linux-androideabi/4.9/include -I/home/minozhao/Android-Kernel-Hacking/OperatingSystemPrj2/kernel/goldfish/arch/arm/include -Iarch/arm/include/generated -Iinclude  -include /home/minozhao/Android-Kernel-Hacking/OperatingSystemPrj2/kernel/goldfish/include/linux/kconfig.h -D__KERNEL__ -mlittle-endian -Iarch/arm/mach-goldfish/include -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -Werror-implicit-function-declaration -Wno-format-security -fno-delete-null-pointer-checks -Os -fno-pic -marm -fno-dwarf2-cfi-asm -mabi=aapcs-linux -mno-thumb-interwork -funwind-tables -D__LINUX_ARM_ARCH__=7 -march=armv7-a -msoft-float -Uarm -Wframe-larger-than=1024 -fno-stack-protector -Wno-unused-but-set-variable -fomit-frame-pointer -g -Wdeclaration-after-statement -Wno-pointer-sign -fno-strict-overflow -fconserve-stack    -D"KBUILD_STR(s)=\#s" -D"KBUILD_BASENAME=KBUILD_STR(Yimin_mm_limits)"  -D"KBUILD_MODNAME=KBUILD_STR(Yimin_mm_limits)" -c -o mm/Yimin_mm_limits.o mm/Yimin_mm_limits.c

source_mm/Yimin_mm_limits.o := mm/Yimin_mm_limits.c

deps_mm/Yimin_mm_limits.o := \
  include/linux/Yimin_struct.h \

mm/Yimin_mm_limits.o: $(deps_mm/Yimin_mm_limits.o)

$(deps_mm/Yimin_mm_limits.o):
