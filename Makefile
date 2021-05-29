# The kernel compile makefile
# This file will build the kernel into the build directory in this file.
# Please set the KERN environment variable before making
.PHONY: all build update config menuconfig clean deep_clean

current_dir = $(shell pwd)
CC=distcc
CXX=distcc

all: build 

build: .build .kernel update
	+make -C ${current_dir}/.kernel O=${current_dir}/.build

update: patch .kernel
	cp -r --remove-destination ${current_dir}/patch/* ${current_dir}/.kernel

.kernel:
	cp -rs $${KERN} ${current_dir}/.kernel
	

config: .build .kernel  update
	+make -C ${current_dir}/.kernel O=${current_dir}/.build goldfish_armv7_defconfig \
	ARCH=arm CROSS_COMPILE=arm-linux-androideabi- 

menuconfig: .build .kernel  update
	+make -C ${current_dir}/.kernel O=${current_dir}/.build menuconfig

clean: .build
	+make -C ${current_dir}/.kernel O=${current_dir}/.build clean
	rm -rf compile_commands.json CMakeLists.txt

deep_clean: .build
	+make -C ${current_dir}/.kernel O=${current_dir}/.build mrproper
	rm -rf .kernel
	rm -rf .build
	rm -rf compile_commands.json CMakeLists.txt

.build:
	mkdir -p ${current_dir}/.build
	
_cmakefile:
# This is a dirty hack used to generate cmakefiles, just for IDE hinting
	+bear -- make all
	../kernel-grok/generate_cmake
