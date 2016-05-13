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

#ifndef SIGNALS_H_
#define SIGNALS_H_


/*
 * Register signals used for quit program.
 * Return the signal descriptor on success or -1 on error.
 */
int register_quit_signals(void);

/*
 * Read signal from the descriptor and check it.
 * Return:
 *          -1 on error;
 *          0 if signal is quit signal;
 *          1 if signal is unknown;
 */
int check_quit_signal(const int fd);


#endif /* SIGNALS_H_ */
