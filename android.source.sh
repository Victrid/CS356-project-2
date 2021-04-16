#!/bin/bash
# use `source ./android.source.sh` to put this into effect
# Exporting NDK and emulator folder
# EDIT THIS WITH YOUR OWN SETTINGS
export NDKL="/home/victrid/AUR/osprj/ndk"
export EMUL="/home/victrid/AUR/osprj/sdk/tools"
export KERN="/home/victrid/AUR/osprj/kernel/goldfish"
export DEST_FOLDER="/data/misc/osprj"

PATH="${NDKL}${PATH:+:${PATH}}"; export PATH;
PATH="${EMUL}${PATH:+:${PATH}}"; export PATH;

# Exporting cross-compiler location for kernel cross-compiling
export XCOMPILERL="${NDKL}/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64/bin"

PATH="${XCOMPILERL}${PATH:+:${PATH}}"; export PATH;

# distcc configuation for faster compilation
if [ -f "/usr/bin/distcc" ]; then
    PATH="/home/victrid/AUR/osprj2/distcc${PATH:+:${PATH}}"; export PATH;
fi
