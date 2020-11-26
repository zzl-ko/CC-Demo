ifneq ($(KERNELRELEASE),)

obj-m := ultrasonic.o

else

MDIR          := $(PWD)
KDIR          ?= /home/kevin/workspace/code/firefly-linux-sdk/kernel
ARCH          ?= arm64
HOST_ARCH     ?= $(shell uname -m | sed -e s/arm.*/arm/ -e s/aarch64.*/arm64/)
CROSS_COMPILE ?= /home/kevin/workspace/code/firefly-linux-sdk/prebuilts/gcc/linux-x86/aarch64/gcc-linaro-6.3.1-2017.05-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-

ifneq  ($(HOST_ARCH), arm)
  ifeq ($(ARCH), arm)
   CROSS_COMPILE ?= arm-linux-gnueabihf-
 endif
endif
ifneq  ($(HOST_ARCH), arm64)
  ifeq ($(ARCH), arm64)
   CROSS_COMPILE ?= aarch64-linux-gnu-
  endif
endif

ifeq ($(HOST_ARCH),$(ARCH))
CROSS_COMPILE :=
endif

modules: FORCE
	$(MAKE) -C $(KDIR) M=$(MDIR) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) modules

install_modules:
	@insmod ultrasonic.ko
	@echo "modules install success"
	@lsmod | grep ultrasonic

uninstall_modules:
	@rmmod ultrasonic

FORCE:
	$(CROSS_COMPILE)gcc ultrasonic_test.c -o ultrasonic_test

clean:
	$(MAKE) -C $(KDIR) M=$(MDIR) clean
	@rm -f *.o.ur-safe

endif