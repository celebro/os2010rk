obj-m := lcd_driver.o
#module-objs := file1.o file2.o

SHELL := /bin/bash

all:
	make ARCH=arm CROSS_COMPILE=arm-linux- -C /home/celebro/celebro/fri/os2/kernel/linux-2.6.27-fri M=/home/celebro/celebro/programming/c/lcd_driver modules

clean:
	rm -f modules.order
	rm -f Module.symvers
	rm -f *.ko
	rm -f *.mod.c
	rm -f *.o
