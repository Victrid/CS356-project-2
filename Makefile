# The kernel compile makefile
# This file will build the kernel into the build directory in this file.
# Please set the KERN environment variable before making
.PHONY: all build update config menuconfig clean deep_clean utils push report

current_dir = $(shell pwd)
CC=distcc
CXX=distcc

all:
	+make build
	+make build_nodebug

build: .kernel update config utils
	./compile_counter.py
	+make -C ${current_dir}/.kernel O=${current_dir}/.build
	cp ${current_dir}/.build/arch/arm/boot/zImage zImages/debug_zImage

build_nodebug: .kernel update utils
	./compile_counter.py
	+make -C ${current_dir}/.kernel O=${current_dir}/.build goldfish_armv7_nodebug_defconfig \
	ARCH=arm CROSS_COMPILE=arm-linux-androideabi- 
	+make -C ${current_dir}/.kernel O=${current_dir}/.build
	cp ${current_dir}/.build/arch/arm/boot/zImage zImages/nodebug_zImage

update: patch .kernel
	cp -rs --remove-destination ${current_dir}/patch/* ${current_dir}/.kernel

.kernel:
	cp -rs $${KERN} ${current_dir}/.kernel
	

config: .build .kernel update
	+make -C ${current_dir}/.kernel O=${current_dir}/.build goldfish_armv7_defconfig \
	ARCH=arm CROSS_COMPILE=arm-linux-androideabi- 

menuconfig: .build .kernel  update
	+make -C ${current_dir}/.kernel O=${current_dir}/.build menuconfig \
	ARCH=arm CROSS_COMPILE=arm-linux-androideabi- 

clean: .build
	+make -C ${current_dir}/.kernel O=${current_dir}/.build clean \
	ARCH=arm CROSS_COMPILE=arm-linux-androideabi- 
	rm -rf compile_commands.json CMakeLists.txt
	rm -rf zImages/debug_zImage zImages/nodebug_zImage
	+make -C utils clean
	+make -C report clean

deep_clean: clean
	+make -C ${current_dir}/.kernel O=${current_dir}/.build mrproper \
	ARCH=arm CROSS_COMPILE=arm-linux-androideabi- 
	rm -rf .kernel
	rm -rf .build
	rm -rf compile_commands.json CMakeLists.txt

.build:
	mkdir -p ${current_dir}/.build
	
utils:
	+make -C utils

push:
	+make -C utils push

report:
	+make -C report

_cmakefile:
# This is a dirty hack used to generate cmakefiles, just for IDE hinting
	+bear -- make all
	../kernel-grok/generate_cmake
