
export ARCH=arm
export CROSS_COMPILE=/opt/280/linux/toolchain/bin/arm-none-linux-gnueabi-

AR=$(CROSS_COMPILE)ar
LD=$(CROSS_COMPILE)ld
CC=$(CROSS_COMPILE)gcc

#BUILD_FLAGS:=-fno-tree-scev-cprop
#LIBPATH :=-lgcc -L/opt/280/linux/toolchain/lib/gcc/arm-none-linux-gnueabi/4.7.3
DEBFLAGS=-02
#KBUILD_CFLAGS := -fno-tree-scev-cprop
PWD := $(shell pwd)
src:=src

obj-m := ifdm.o
ifdm-objs := ifd.o

KERNELDIR?=/opt/280/linux/kernel/

modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD)
	@echo 'Module ifd_ICC.ko was already generated'
#	@cp *.ko /mnt/hgfs/sharefiles

clean:
	rm -rf $(srcdir)*.o $(srcdir)*.mod.o $(srcdir)*.ko $(srcdir)*.mod.c \
	*.o *.mod.o *.ko *.mod.c *.symvers *.order .*.*.cmd .tmp_versions
