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

#ifndef PRIVILEGES_H_
#define PRIVILEGES_H_


/*
 * Save root and user privileges.
 * Return 0 on success and >0 on error.
 */
int init_privileges(const char *user_name);

/*
 * Restore root privileges.
 * Return 0 on success and >0 on error.
 */
int set_root_privileges(void);

/*
 * Restore user privileges back after last call of `set_root_privileges()'.
 * Return 0 on success and >0 on error.
 */
int set_user_privileges(void);

/*
 * Free all allocated resources.
 */
void free_privileges(void);


#endif /* PRIVILEGES_H_ */
