/*
    This file is part of fspupsmon.

    Copyright (C) 2016 Vadim Kuznetsov <vimusov@gmail.com>

    fspupsmon is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    fspupsmon is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with fspupsmon.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>

#include "log.h"


#define PORT_SPEED (B2400)  /* port speed according to Megatec protocol specification */


int open_port(const char* port)
{
    int mcs = 0;
    struct termios opts;

    const int fd = open(port, O_RDWR | O_CLOEXEC);
    if (fd < 0) {
        LOG_E("unable open port %s, error '%m'", port);
        return -1;
    }

    if (tcflush(fd, TCIOFLUSH) < 0) {
        LOG_E("unable to flush port %s, error '%m'", port);
        goto on_error;
    }

    if (tcgetattr(fd, &opts)) {
        LOG_E("unable to get settings from port %s, error '%m'", port);
        goto on_error;
    }

    if (cfsetispeed(&opts, PORT_SPEED)) {
        LOG_E("unable to set input speed on port %s", port);
        goto on_error;
    }

    if (cfsetospeed(&opts, PORT_SPEED)) {
        LOG_E("unable to set output speed on port %s", port);
        goto on_error;
    }

    opts.c_cflag = (opts.c_cflag & ~CSIZE) | CS8;
    opts.c_cflag |= CLOCAL | CREAD;
    opts.c_cflag &= ~(PARENB | PARODD);
    opts.c_cflag &= ~CRTSCTS;
    opts.c_cflag &= ~CSTOPB;
    opts.c_iflag = IGNBRK;
    opts.c_iflag &= ~(IXON | IXOFF | IXANY);
    opts.c_lflag = 0;
    opts.c_oflag = 0;
    opts.c_cc[VTIME] = 1;
    opts.c_cc[VMIN] = 60;

    if (tcsetattr(fd, TCSANOW, &opts) < 0) {
        LOG_E("unable to apply setting to port %s, error '%m'", port);
        goto on_error;
    }

    if (ioctl(fd, TIOCMGET, &mcs) < 0) {
        LOG_E("ioctl(TIOCMGET), error '%m'");
        goto on_error;
    }

    mcs |= TIOCM_RTS;

    if (ioctl(fd, TIOCMSET, &mcs) < 0) {
        LOG_E("ioctl(TIOCMSET), error '%m'");
        goto on_error;
    }

    if (tcgetattr(fd, &opts)) {
        LOG_E("unable get settings from port %s, error '%m'", port);
        goto on_error;
    }

    opts.c_cflag &= ~CRTSCTS;

    if (tcsetattr(fd, TCSANOW, &opts) < 0) {
        LOG_E("unable apply setting to port %s, error '%m'", port);
        goto on_error;
    }

    LOG_I("port %s successfully opened and configured", port);

    return fd;

on_error:

    if (close(fd))
        LOG_E("unable close port %s, error '%m'", port);

    return -1;
}
