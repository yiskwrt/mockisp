## configuration to launch builder
KERNEL_DIR=../linux

build_local:
	make -C $(KERNEL_DIR) M=$(PWD) ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- modules

clean_local:
	rm -f *.o *.ko *.mod *.mod.c modules.order Module.symvers *.dtbo

## build configuration to work with Linux source tree
obj-m := mockisp.o

mockisp-objs := mockisp-dev.o mockisp-capture.o mockisp-resize.o

dtbo-y += mockisp.dtbo
always-y := $(dtbo-y)
