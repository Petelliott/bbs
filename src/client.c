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


/*
    reads a line from a file descripter as a null terminated string.
    buff is required to be consistent and should be MAX_LINE_SIZE
    buff_idx should be initialized to 0
    the newline will not be included in the returned string
*/
char *readline(int sock_fd, char *buff, size_t *buff_idx) {
    struct pollfd polls;
    polls.fd = sock_fd;
    polls.events = POLLIN | POLLPRI;

    while (1) {
        poll(&polls, 1, -1);

        int read_len = read(sock_fd, buff+*buff_idx, MAX_LINE_SIZE-*buff_idx);

        if (read_len < 0) {
            perror("read(2)");
        }

        for (size_t i = *buff_idx; i < (*buff_idx+read_len); ++i) {
            if (buff[i] == '\r') {
                char *output = malloc(i+1);
                memcpy(output, buff, i);
                output[i] = 0;

                *buff_idx = 0;
                return output;
            }
        }
        *buff_idx += read_len;
    }
}


// TODO: this is a terrible bandaid
void skip_telnet(int fd) {
    char telnet_data[28];
    read(fd, telnet_data, 28);
}


void *client_thread(void *args) {

    int sock_fd = ((struct ct_args *) args)->sock_fd;
    struct logindb *l_db = ((struct ct_args *) args)->l_db;
    struct post_fds *posts = ((struct ct_args *) args)->posts;
    free(args);

    skip_telnet(sock_fd); //TODO: remove asap

    char linebuff[1024];
    size_t buff_idx = 0;

    // username and password prompt
    dprintf(sock_fd, "user: ");
    char *username = readline(sock_fd, linebuff, &buff_idx);
    dprintf(sock_fd, "pass: ");
    char *password = readline(sock_fd, linebuff, &buff_idx);

    if (!login(l_db, username, password)) {
        free(username);
        free(password);
        close(sock_fd);
        return NULL;
    }

    printf("%s has logged in\n", username);
    dprintf(sock_fd, "welcome, %s\n", username);

    free(username);
    free(password);
    close(sock_fd);

    return NULL;
}
