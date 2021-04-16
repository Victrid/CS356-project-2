# The kernel compile makefile
# This file will build the kernel into the build directory in this file.
# Please set the KERN environment variable before making
.PHONY: all build setup_kernel config menuconfig clean deep_clean

current_dir = $(shell pwd)
CC=distcc
CXX=distcc

all: build 

build: .build .kernel
	+make -C ${current_dir}/.kernel O=${current_dir}/.build

.kernel: patch
	cp -rs $${KERN} ${current_dir}/.kernel
	cp -r --remove-destination ${current_dir}/patch/* ${current_dir}/.kernel

config: .build .kernel
	+make -C ${current_dir}/.kernel O=${current_dir}/.build goldfish_armv7_defconfig \
	ARCH=arm CROSS_COMPILE=arm-linux-androideabi- 

menuconfig: .build .kernel
	+make -C ${current_dir}/.kernel O=${current_dir}/.build menuconfig

clean: .build
	+make -C ${current_dir}/.kernel O=${current_dir}/.build clean
	rm -rf .kernel
	rm -rf .build

deep_clean: .build
	+make -C ${current_dir}/.kernel O=${current_dir}/.build mrproper
	rm -rf .kernel
	rm -rf .build

.build:
	mkdir -p ${current_dir}/.build
