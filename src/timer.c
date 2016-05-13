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

#include <unistd.h>
#include <sys/timerfd.h>

#include "log.h"


int create_timer(const unsigned int timeout)
{
    const int fd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC);
    if (fd < 0) {
        LOG_E("unable to create timer fd, error '%m'");
        return -1;
    }

    struct itimerspec tm = {
        .it_interval = {
            .tv_sec = timeout,
            .tv_nsec = 0
        },
        .it_value = {
            .tv_sec = timeout,
            .tv_nsec = 0
        }
    };

    if (!timerfd_settime(fd, 0, &tm, NULL))
        return fd;

    LOG_E("unable to set timerfd value, error '%m'");

    if (close(fd))
        LOG_E("unable to close timerfd #%d, error '%m'", fd);

    return -1;
}
