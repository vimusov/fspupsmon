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

#include "log.h"


void init_log(const int debug_mode)
{
    int flags = LOG_PID | LOG_CONS;

    if (debug_mode)
        flags |= LOG_PERROR;

    openlog("fspupsmon", flags, LOG_USER);

    if (debug_mode)
        setlogmask(LOG_UPTO(LOG_DEBUG));
    else
        setlogmask(LOG_UPTO(LOG_INFO));
}
