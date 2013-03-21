
CROSS_COMPILE=/opt/acr910/arm-2009q1/bin/arm-none-linux-gnueabi-
AR=$(CROSS_COMPILE)ar
ARCH=arm
LD=$(CROSS_COMPILE)ld
CC=$(CROSS_COMPILE)gcc
DEBFLAGS=-O2
PWD:=$(shell pwd)

objects=app.o
app.o:
  $(CC) -Wall  -lpthread  -o  app app.c
	@echo 'app was already generated'
	@cp app /mnt/hgfs/sharefiles
clean:
	rm -rf $(objects)cd
