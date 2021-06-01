pkg=$(/home/victrid/AUR/osprj/sdk/build-tools/23.0.2/aapt dump badging $1|awk -F" " '/package/ {print $2}'|awk -F"'" '/name=/ {print $2}')
act=$(/home/victrid/AUR/osprj/sdk/build-tools/23.0.2/aapt dump badging $1|awk -F" " '/launchable-activity/ {print $2}'|awk -F"'" '/name=/ {print $2}')
adb shell am start -n $pkg/$act 
