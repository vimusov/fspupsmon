/*
    fspupsmon - monitoring daemon for UPSs made by FSP company.
    It looks after UPS and logs its status. If UPS is offline
    more than specified time the system would be shutdown gently.

    Copyright (C) 2016 Vadim Kuznetsov <vimusov@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <poll.h>
#include <time.h>
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>

#include "log.h"
#include "port.h"
#include "privileges.h"
#include "protocol.h"
#include "signals.h"
#include "timer.h"


/*
 * Update UPS status and run `shutdown' if UPS is offline for too long.
 * Return:
 *          -1 on error;
 *          0 if the system is going to shutdown;
 *          1 if UPS is online;
 */
static int update_status(const int fd, const unsigned int delay)
{
    static uint64_t prev_time = 0;

    ups_status status = parse_response(fd);

    switch (status) {
        case UPS_ONLINE:
            if (prev_time) {
                LOG_I("UPS became online, system shutdown canceled");
                prev_time = 0;
            }
            else
                LOG_D("UPS is online");
            return 1;

        case UPS_OFFLINE:
            break;

        default:
            return -1;
    }

    struct timespec tm;

    if (clock_gettime(CLOCK_MONOTONIC, &tm)) {
        LOG_E("unable to get current time, error '%m'");
        return -1;
    }

    uint64_t cur_time = tm.tv_sec;

    if (!prev_time) {
        prev_time = cur_time;
        LOG_I("UPS became offline, %u sec left before system shutdown", delay);
        return 1;
    }

    uint64_t delta = cur_time - prev_time;

    if (delta < (uint64_t) delay) {
        LOG_I("UPS is offline, %" PRIu64 " sec left before system shutdown", delay - delta);
        return 1;
    }

    prev_time = 0;
    LOG_I("shutdown delay is over, going to shutdown system");

    if (set_root_privileges())  /* return root privileges back in order to run `shutdown' */
        return -1;

    int ret = system("shutdown");
    if (ret)
        LOG_E("unable to execute command 'shutdown', ret=%d, error='%m'", ret);
    else
        LOG_I("shutdown in progress, 1 minute left");

    set_user_privileges();  /* drop privileges back to the user */

    return ret;
}


/*
 * Process events from descriptors.
 */
static void process_events(const int sig_fd, const int port_fd, const int timer_fd, const unsigned int delay)
{
    int sleeping = 1;
    uint64_t unused = 0;
    struct pollfd fds[2];

    fds[0].fd = sig_fd;
    fds[0].events = POLLIN;

    /*
     * This element is used for two purposes:
     *      1) to make delays between UPS status updates;
     *      2) to read and process current UPS status;
     * First we wait for a timer event. Then we update UPS status. And repeat.
     */
    fds[1].fd = timer_fd;
    fds[1].events = POLLIN;

    LOG_I("start processing events");

    for (;;) {
        fds[0].revents = 0;
        fds[1].revents = 0;

        if (poll(fds, (sizeof(fds) / sizeof(fds[0])), -1) < 0) {
            LOG_E("poll() return error '%m'");
            continue;
        }

        if (fds[0].revents & POLLIN) {
            if (!check_quit_signal(sig_fd))
                break;
        }

        if (fds[1].revents & POLLIN) {
            if (sleeping) {
                read(timer_fd, &unused, sizeof(unused));  /* we don't care about this data */
                fds[1].fd = port_fd;
                fds[1].events = POLLOUT;
                sleeping = 0;
            }
            else {
                if (!update_status(port_fd, delay))
                    break;
                fds[1].fd = timer_fd;
                fds[1].events = POLLIN;
                sleeping = 1;
            }
            continue;
        }

        if (fds[1].revents & POLLOUT) {
            if (!send_request(port_fd))
                fds[1].events = POLLIN;
        }
    }
}


int main(int argc, char** argv)
{
    int opt;
    int sig_fd = -1;
    int port_fd = -1;
    int timer_fd = -1;
    int exit_code = EXIT_FAILURE;
    int debug_mode = 0;
    const char *port = "/dev/ttyS0";
    unsigned int delay = 10;
    unsigned int timeout = 5;
    const char *user_name = NULL;

    while ((opt = getopt(argc, argv, "hdi:p:s:u:")) > 0)
        switch (opt) {
            case 'd':
                debug_mode = 1;
                break;

            case 'i':
                timeout = strtoul(optarg, NULL, 10);
                if (timeout < 1 || timeout > 60) {
                    fprintf(stderr, "Error: Invalid query interval value %u, must be in [1..60]\n", timeout);
                    return EXIT_FAILURE;
                }
                break;

            case 'p':
                port = optarg;
                break;

            case 's':
                delay = strtoul(optarg, NULL, 10);
                if (delay < 1 || delay > 60) {
                    fprintf(stderr, "Error: Invalid shutdown delay value %u, must be in [1..60]\n", delay);
                    return EXIT_FAILURE;
                }
                break;

            case 'u':
                user_name = optarg;
                break;

            default:
                printf(
                    "Usage: fspupsmon [-h] [-d] [-i <SEC>] [-p <PORT>] [-s <MIN>] [-u <USER>]\n"
                    "Arguments:\n"
                    "    -h: show this help;\n"
                    "    -d: turn on debug mode;\n"
                    "    -i <SEC>: query interval, seconds (default %u)\n"
                    "    -p <PORT>: serial port (default %s);\n"
                    "    -s <MIN>: delay before shutdown, minutes (default %u)\n"
                    "    -u <USER>: drop privileges to specified user;\n",
                    timeout, port, delay
                );
                return EXIT_FAILURE;
        }

    init_log(debug_mode);

    if (user_name != NULL && init_privileges(user_name))
        goto on_error;

    sig_fd = register_quit_signals();
    if (sig_fd < 0)
        goto on_error;

    port_fd = open_port(port);
    if (port_fd < 0)
        goto on_error;

    timer_fd = create_timer(timeout);
    if (timer_fd < 0)
        goto on_error;

    process_events(sig_fd, port_fd, timer_fd, delay * 60);
    exit_code = EXIT_SUCCESS;

on_error:

    if (timer_fd > 0 && close(timer_fd))
        LOG_E("unable to close timerfd #%d, error '%m'", timer_fd);

    if (port_fd > 0 && close(port_fd))
        LOG_E("unable to close port %s, error '%m'", port);

    if (sig_fd > 0 && close(sig_fd))
        LOG_E("unable to close signalfd #%d, error '%m'", sig_fd);

    free_privileges();

    LOG_I("shutdown completed");

    closelog();

    return exit_code;
}
