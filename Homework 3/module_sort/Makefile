LINUXDIR := $(HOME)/Desktop/GIT_AESD/buildroot/output/build/linux-4796173fc58688055a99a1cef19a839174067220
COMPILEDIR := $(HOME)/Desktop/GIT_AESD/buildroot/output/host/bin/

obj-m += sort_module.o

all:
	make ARCH=arm CROSS_COMPILE=$(COMPILEDIR)arm-buildroot-linux-uclibcgnueabihf- \-C $(LINUXDIR) M=$(shell pwd) modules

clean:
	make -C $(LINUXDIR) M=$(shell pwd) clean
