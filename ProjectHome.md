A simple network bandwidth monitor inspired by the suckless philosophy.

It's only purpose is to output the current used bandwith on a specified network interface in a specific direction.

Usage examples:

```
    ./bwm --interface eth0 --download
    ./bwm --interface wlan0 --upload > txKBps
```

You can use the produced output as you like, for example redirecting the output to a file and then showing the used bandwith on the [wmii](http://wmii.suckless.org/) status bar (see image below).

![http://imgur.com/Z3ffh.png](http://imgur.com/Z3ffh.png)