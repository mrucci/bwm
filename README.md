# bwm
A simple network bandwidth monitor inspired by the suckless philosophy.

It's only purpose is to output the current used bandwith on a specified network interface in a specific direction.

Usage examples:

    ./bwm --interface eth0 --download
    ./bwm --interface wlan0 --upload > txKBps
