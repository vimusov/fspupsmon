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

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "log.h"
#include "protocol.h"


/*
    The FSP protocol is definitely based on Megatec protocol but with some differences:
    - request:
        QS\r
    - response:
        (229.2 229.2 229.2 014 50.1 27.6 --.- 00001001\r  // UPS online (high bit is NOT set)
        (012.3 229.7 220.2 014 50.1 24.6 --.- 10001001\r  // UPS offline (high bit is SET)
*/


#define REQUEST ("QS\r")
#define STATUS_FMT ("%c[01][01][01][01][01][01][01]")
#define REQUEST_SIZE (sizeof(REQUEST) - 1)


int send_request(const int fd)
{
    if (write(fd, REQUEST, REQUEST_SIZE) == REQUEST_SIZE) {
        LOG_D("request has been sent");
        return 0;
    }
    else {
        LOG_E("unable to send request, error '%m'");
        return 1;
    }
}


ups_status parse_response(const int fd)
{
    char *delim;
    char response[64];

    memset(response, 0, sizeof(response));

    ssize_t size = read(fd, response, sizeof(response) - 1);
    if (size <= 0) {
        LOG_E("read() return %zd, error '%m'", size);
        return INVALID_RESPONSE;
    }

    delim = strrchr(response, '\r');  /* terminate string with zero */
    if (delim != NULL)
        *delim = '\0';

    LOG_D("response='%s'", response);

    delim = strrchr(response, ' ');  /* UPS status is after last space */
    if (delim == NULL) {
        LOG_E("unable to find last space in response, invalid response");
        return INVALID_RESPONSE;
    }

    char *status_line = delim + 1;
    LOG_D("status='%s'", status_line);

    char status = 0;

    if (sscanf(status_line, STATUS_FMT, &status) != 1) {
        LOG_E("invalid status line '%s', no matches found", status_line);
        return INVALID_RESPONSE;
    }

    switch (status) {
        case '0':
            return UPS_ONLINE;

        case '1':
            return UPS_OFFLINE;

        default:
            if (isprint(status))
                LOG_E("invalid UPS status code '%c'", status);
            else
                LOG_E("invalid UPS status code 0x%.2X", status);
            break;
    }

    return INVALID_RESPONSE;
}
