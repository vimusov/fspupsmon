Summary
=======

fspupsmon - monitoring daemon for UPSs made by FSP company.
It looks after UPS and logs its status. If UPS is offline
more than specified time the system would be shutdown gently.

Supported hardware
==================

* FSP EP-1000

Usage
=====

```
Usage: fspupsmon [-h] [-d] [-i <SEC>] [-p <PORT>] [-s <MIN>] [-u <USER>]
Arguments:
    -h: show this help;
    -d: turn on debug mode;
    -i <SEC>: query interval, seconds (default 5);
    -p <PORT>: serial port (default /dev/ttyS0);
    -s <MIN>: delay before shutdown, minutes (default 10);
    -u <USER>: drop privileges to specified user;
```

Build and install
=================

```
make
make DESTDIR=... install
```

License
=======

GPL
