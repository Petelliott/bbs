/*
main.c
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
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <time.h>
#include "client.h"
#include <poll.h>

#define TELENT_PORT 23

#define MAX_CONNS 32


int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket(2)");
        return 1;
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(TELENT_PORT);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    int opt_val = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof(opt_val));

    if (bind(server_fd, (struct sockaddr *) &server, sizeof(server)) < 0) {
        perror("bind(2)");
        return 1;
    }

    if (listen(server_fd, MAX_CONNS) < 0) {
        perror("listen(2)");
        return 1;
    }

    srand(time(NULL)); // used by logindb

    struct logindb l_db;
    logindb_init(&l_db, "bbs_login.db");
    struct post_fds p_db;
    posts_db_init(&p_db, "bbs_post_meta.db", "bbs_post_db.db");

    while (1) {
        struct sockaddr_in client;
        socklen_t client_len = sizeof(client);
        int client_fd = accept(server_fd, (struct sockaddr *) &client, &client_len);

        if (client_fd < 0) {
            perror("accept(2)");
        }

        struct ct_args *args = malloc(sizeof(struct ct_args));
        args->sock_fd = client_fd;
        args->l_db = &l_db;
        args->posts = &p_db;

        pthread_t new_cli_thread;
        pthread_create(&new_cli_thread, NULL, &client_thread, args);
    }

    return 0;
}
