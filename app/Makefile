
export ARCH=arm
export PATH=/opt/280/linux/toolchain/bin:$PATH

CROSS_COMPILE=arm-none-linux-gnueabi-

AR=$(CROSS_COMPILE)ar
LD=$(CROSS_COMPILE)ld
CC=$(CROSS_COMPILE)gcc

LDFLAGS=-static

DEBFLAGS=-O2
PWD:=$(shell pwd)

objects=app.o
app.o:
	$(CC) -Wall  -lpthread  -o  app app.c -static
#	@echo 'app was already generated'
#	cp app /mnt/hgfs/sharefiles
clean:
	rm -rf $(objects)

