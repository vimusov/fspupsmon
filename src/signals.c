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

#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/signalfd.h>

#include "log.h"
#include "signals.h"


static const int quit_signals[] = {
    SIGINT,
    SIGTERM
};
#define QUIT_SIGNALS_COUNT (sizeof(quit_signals) / sizeof(quit_signals[0]))


int register_quit_signals(void)
{
    sigset_t mask;

    if (sigfillset(&mask)) {
        LOG_E("unable to fill signals mask, error '%m'");
        return -1;
    }

    if (sigprocmask(SIG_BLOCK, &mask, NULL)) {
        LOG_E("unable to block signals, error '%m'");
        return -1;
    }

    if (sigemptyset(&mask)) {
        LOG_E("unable to empty signals mask, error '%m'");
        return -1;
    }

    for (size_t i = 0; i < QUIT_SIGNALS_COUNT; i++) {
        int signum = quit_signals[i];

        if (sigaddset(&mask, signum)) {
            LOG_E("unable to add signal %d, error '%m'", signum);
            return -1;
        }
        else
            LOG_D("signal #%d ('%s') added", signum, strsignal(signum));
    }

    const int fd = signalfd(-1, &mask, SFD_CLOEXEC);
    if (fd < 0)
        LOG_E("unable to create signalfd, error '%m'");

    return fd;
}


int check_quit_signal(const int fd)
{
    struct signalfd_siginfo sig_info;

    if (read(fd, &sig_info, sizeof(sig_info)) != sizeof(sig_info)) {
        LOG_E("broken sig_info structure, error '%m'");
        return -1;
    }

    const int signum = sig_info.ssi_signo;

    for (size_t i = 0; i < QUIT_SIGNALS_COUNT; i++)
        if (signum == quit_signals[i]) {
            LOG_I("got signal #%d ('%s'), shutting down", signum, strsignal(signum));
            return 0;
        }

    LOG_E("unknown signal #%d ('%s'), ignored", signum, strsignal(signum));

    return 1;
}
