# The kernel compile makefile
# This file will build the kernel into the build directory in this file.
# Please set the KERN environment variable before making

.PHONY: all build update config menuconfig clean deep_clean utils push report

current_dir = $(shell pwd)

# You may set this to fit your system configuation.
CC?=distcc
CXX?=distcc
Student_number?=519021911045

all:
	+make build
	+make build_nodebug

# Build script

## build the kernel with debug info
build: .kernel update config utils
	./compile_counter.py
	+make -C ${current_dir}/.kernel O=${current_dir}/.build
	cp ${current_dir}/.build/arch/arm/boot/zImage zImages/debug_zImage

## build the kernel without debug info
build_nodebug: .kernel update utils
	./compile_counter.py
	+make -C ${current_dir}/.kernel O=${current_dir}/.build goldfish_armv7_nodebug_defconfig \
	ARCH=arm CROSS_COMPILE=arm-linux-androideabi- 
	+make -C ${current_dir}/.kernel O=${current_dir}/.build
	cp ${current_dir}/.build/arch/arm/boot/zImage zImages/nodebug_zImage

utils:
	+make -C utils

push:
	+make -C utils push

# Kernel configuration
config: .build .kernel update
	+make -C ${current_dir}/.kernel O=${current_dir}/.build goldfish_armv7_defconfig \
	ARCH=arm CROSS_COMPILE=arm-linux-androideabi- 

menuconfig: .build .kernel  update
	+make -C ${current_dir}/.kernel O=${current_dir}/.build menuconfig \
	ARCH=arm CROSS_COMPILE=arm-linux-androideabi- 

# Patching the kernel and create directories.
## With this we can keep the original kernel clean.

.kernel:
	cp -rs $${KERN} ${current_dir}/.kernel

.build:
	mkdir -p ${current_dir}/.build

update: patch .kernel
	cp -rs --remove-destination ${current_dir}/patch/* ${current_dir}/.kernel

# Clean current build
clean: .build
	+make -C ${current_dir}/.kernel O=${current_dir}/.build clean \
	ARCH=arm CROSS_COMPILE=arm-linux-androideabi- 
	rm -rf handout handout.tar.gz Prj2+${Student_number}.zip
	rm -rf zImages/debug_zImage zImages/nodebug_zImage
	+make -C utils clean
	+make -C report clean

# Clean configurations
deep_clean: clean
	+make -C ${current_dir}/.kernel O=${current_dir}/.build mrproper \
	ARCH=arm CROSS_COMPILE=arm-linux-androideabi- 
	rm -rf .kernel .build handout.tar.gz

# Report stuff

handout: report
	mkdir -p ${current_dir}/handout
	git archive HEAD -o ${current_dir}/handout.tar.gz
	tar -xzvf handout.tar.gz --directory=${current_dir}/handout
	rm -rf handout.tar.gz
	cp Prj2README ${current_dir}/handout/Prj2README
	cp ${current_dir}/report/report.pdf ${current_dir}/handout/report.pdf
	(cd ${current_dir}/handout; zip -r ../Prj2+${Student_number}.zip  .)

report:
	+make -C report
