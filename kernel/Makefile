ifneq ($(KERNELRELEASE),)
  obj-m += my.o
  my-objs := test.o
else
  ARCH := mips
  CROSS_COMPILE := mips-mti-linux-gnu-
  KDIR := /home/zhangyinjun/linux-4.14
  PWD := $(shell pwd)
default:
	make -C $(KDIR) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) M=$(PWD) modules
local:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	rm -rf *.ko *.o *.mod.c *.mod.o *.order *.symvers *.o.rc .*.cmd .tmp_versions
endif
