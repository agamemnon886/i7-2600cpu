obj-m := i7_2600cpu.o

KERNELDIR ?= /lib/modules/$(shell uname -r)/build
#KERNELDIR ?= /home/giskar/mod/linux-stable
PWD       := $(shell pwd)

all install:
	$(MAKE) -C $(KERNELDIR) M=$(PWD)

clean clobber:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
