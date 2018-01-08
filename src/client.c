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
        poll(&polls, 1, 30);

        int read_len = read(sock_fd, buff+*buff_idx, MAX_LINE_SIZE-*buff_idx);

        if (read_len <= 0) {
            return NULL;
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

// TODO: FIGURE OUT TELNET LINEMODES
// const char echo_off[] = {255, 251, 1};
// const char echo_on[]  = {255, 252, 1, 255, 254, 34};
//
//
// char *read_password_telnet(int sock_fd) {
//     char *password = malloc(512); // TODO: magic number
//     size_t p_idx = 0;
//
//     char nothing[6];
//     write(sock_fd, echo_off, 3);
//     read(sock_fd, nothing, 3);
//
//     char c;
//     while (1) {
//         read(sock_fd, &c, 1);
//         if (c == '\r') {
//             read(sock_fd, &c, 1); // expect newline
//             dprintf(sock_fd, "\r\n");
//             break;
//         }
//
//         write(sock_fd, "*", 1);
//         password[p_idx] = c;
//         ++p_idx;
//     }
//
//     password[p_idx] = 0;
//     write(sock_fd, echo_on, 6);
//     read(sock_fd, nothing, 6);
//     return password;
// }


// TODO: this is a terrible bandaid
void skip_telnet(int fd) {
    char telnet_data[28];
    read(fd, telnet_data, 28);
}


/*
    client thread is a pthread running client telnet interactions
    args is of type struct ct_args *
    the return value is always NULL
*/
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

    if (username == NULL || password == NULL) {
        free(username);
        free(password);
        return NULL;
    }

    if (!login(l_db, username, password)) {
        // try to create a new account
        if (add_login(l_db, username, password)) {
            dprintf(sock_fd, "creating account...\r\n");
        } else {
            dprintf(sock_fd, "invalid username and password\r\n");
            free(username);
            free(password);
            close(sock_fd);
            return NULL;
        }
    }

    printf("%s has logged in\n", username);
    dprintf(sock_fd, "welcome, %s\n", username);


    while (1) { // COMMAND LOOP
        dprintf(sock_fd, "> ");

        char *command = readline(sock_fd, linebuff, &buff_idx);

        if (command == NULL) {
            printf("%s has disconnected\n", username);
            break;
        }

        char *saveptr;
        char *op = strtok_r(command, " ", &saveptr);

        if (strncmp(op, "write", 5) == 0) { // WRITE COMMAND
            int n_newlines = 0;
            char post_txt[32768];
            size_t idx = 0;

            dprintf(sock_fd, "title: ");
            char *title = readline(sock_fd, linebuff, &buff_idx);

            while (n_newlines < 2) {
                char *line = readline(sock_fd, linebuff, &buff_idx);
                size_t line_len = strlen(line);
                memcpy(post_txt+idx, line, line_len);
                memcpy(post_txt+idx+line_len, "\r\n", 2);

                idx += line_len + 2;

                if (line_len == 0) {
                    ++n_newlines;
                } else {
                    n_newlines = 0;
                }
                free(line);
            }

            idx -= 4; // remove the last two newlines

            post(posts, username, title, post_txt, idx);
            free(title);

        } else if (strncmp(op, "read", 4) == 0) { // READ COMMAND
            char *arg1 = strtok_r(NULL, " ", &saveptr);
            unsigned long post_num;
            if (arg1 == NULL) {
                post_num = post_head(posts);
            } else {
                post_num = atoi(arg1);
            }


            struct meta_block block;
            char *post_txt = get_post(posts, &block, post_num);

            dprintf(sock_fd, "\e[1m#%lu: %s '%s' %li\r\n\e[0m", post_num, block.name, block.title, block.timestamp);
            write(sock_fd, post_txt, block.len);
            free(post_txt);

        } else if (strncmp(op, "exit", 4) == 0) { // EXIT COMMAND
            printf("%s has left\n", username);
            free(command);
            break;

        } else if (strncmp(op, "list", 4) == 0) { // LIST COMMAND
            char *arg1 = strtok_r(NULL, " ", &saveptr);
            size_t n_posts;
            if (arg1 == NULL) {
                n_posts = 10;
            } else {
                n_posts = atoi(arg1);
            }

            long post_num = post_head(posts);

            for (ssize_t i = 0; i < n_posts && post_num >= i; ++i) {
                struct meta_block block;
                get_post_meta(posts, &block, post_num-i);
                dprintf(sock_fd, "#%lu: %s '%s' %li\r\n", post_num-i, block.name, block.title, block.timestamp);
            }
        }

        free(command);
    }

    free(username);
    free(password);
    close(sock_fd);

    return NULL;
}
