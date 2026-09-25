obj-m += qemu_char.o

KDIR := /home/ergozee/mycode/buildroot-2025.02.15/output/build/linux-6.6.48
CROSS_COMPILE := /home/ergozee/mycode/buildroot-2025.02.15/output/host/bin/arm-buildroot-linux-gnueabihf-

all:
	$(MAKE) -C $(KDIR) M=$(CURDIR) ARCH=arm CROSS_COMPILE=$(CROSS_COMPILE) modules

clean:
	$(MAKE) -C $(KDIR) M=$(CURDIR) clean
