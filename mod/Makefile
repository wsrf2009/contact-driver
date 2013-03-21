
CROSS_COMPILE=/opt/acr910/arm-eabi-4.4.3/bin/arm-linux-
AR=$(CROSS_COMPILE)ar
export ARCH=arm
LD=$(CROSS_COMPILE)ld
CC=$(CROSS_COMPILE)gcc
DEBFLAGS=-02
PWD := $(shell pwd)
src:=src

obj-m := ifdm.o
ifdm-objs := at83c26.o common.o gpio.o ifd.o atr.o param.o t0.o t1.o icc.o timer.o uart.o

KERNELDIR?=/opt/acr910/kernel/

modules:
  $(MAKE) -C $(KERNELDIR) M=$(PWD)
	@echo 'Module ifd_ICC.ko was already generated'
	@cp *.ko /mnt/hgfs/sharefiles

clean:
	rm -rf $(srcdir)*.o $(srcdir)*.mod.o $(srcdir)*.ko $(srcdir)*.mod.c \
	*.o *.mod.o *.ko *.mod.c *.symvers *.order
