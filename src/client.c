/*
client.c
Copyright (C) 2017  Peter Elliott

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as published
by the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "client.h"
#include <stdlib.h>
#include <poll.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>


char *readline(int sock_fd, char *buff, size_t *buff_idx) {
    struct pollfd polls;
    polls.fd = sock_fd;
    polls.events = POLLIN | POLLPRI;

    while (1) {
        poll(&polls, 1, -1);

        int res = read(sock_fd, buff+*buff_idx, MAX_LINE_SIZE-*buff_idx);

        if (res < 0) {
            perror("read(2)");
        }

        for (size_t i = *buff_idx; i < (*buff_idx+res); ++i) {
            if (buff[i] == '\n') {
                char *output = malloc(i+2);
                memcpy(output, buff, i+1);
                output[i+1] = 0;

                *buff_idx = 0;
                return output;
            }
        }
        *buff_idx += res;
    }
}


void client_thread(int sock_fd, struct logindb *l_db, struct post_fds* posts) {

}
