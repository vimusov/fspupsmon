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

#ifndef PROTOCOL_H_
#define PROTOCOL_H_


typedef enum {
    UPS_ONLINE,
    UPS_OFFLINE,
    INVALID_RESPONSE
}
ups_status;


/*
 * Send the request to the UPS.
 * Return 0 on success and >0 on error.
 */
int send_request(const int fd);

/*
 * Read and parse the reposponse from the UPS.
 * Return current UPS status (see above).
 */
ups_status parse_response(const int fd);


#endif /* PROTOCOL_H_ */
