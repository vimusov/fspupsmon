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

#ifndef LOG_H_
#define LOG_H_

#include <syslog.h>


#define LOG_E(fmt, ...) syslog(LOG_ERR,   "E: " fmt "\n", ## __VA_ARGS__)
#define LOG_I(fmt, ...) syslog(LOG_INFO,  "I: " fmt "\n", ## __VA_ARGS__)
#define LOG_D(fmt, ...) syslog(LOG_DEBUG, "D: " fmt "\n", ## __VA_ARGS__)


void init_log(const int debug_mode);


#endif /* LOG_H_ */
