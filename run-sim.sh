#!/bin/bash
set -e
if [ -f "avd.lock" ]; then
exit 1
fi
touch avd.lock
buildimg=zImages/debug_zImage
window=-no-window
if [ "$1" == "-n" ]; then
buildimg=zImages/nodebug_zImage
fi
if [ "$1" == "-nw" ]; then
buildimg=zImages/nodebug_zImage
window=
fi
if [ "$1" == "-w" ]; then
window=
fi
trap "rm -f avd.lock .load*" HUP INT PIPE QUIT TERM
emulator -avd OSPrj-519021911045 $window -kernel $buildimg -no-boot-anim -show-kernel -noaudio
rm -f avd.lock .load*
