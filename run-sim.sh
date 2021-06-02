#!/bin/bash
set -e
if [ -f "avd.lock" ]; then
exit 1
fi
touch avd.lock
buildimg=zImages/debug_zImage
if [ "$1" == "--nodebug" ]; then
buildimg=zImages/nodebug_zImage
fi
trap "rm -f avd.lock .load*" HUP INT PIPE QUIT TERM
emulator -avd OSPrj-519021911045 -no-window -kernel $buildimg -no-boot-anim -show-kernel -noaudio
rm -f avd.lock .load*
