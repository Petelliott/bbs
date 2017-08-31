/*
client.h
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
#ifndef _CLIENT_H
#define _CLIENT_H

#include "posts.h"
#include "login.h"

#define MAX_LINE_SIZE 1024

struct ct_args {
    int sock_fd;
    struct logindb *l_db;
    struct post_fds *posts;
};

void *client_thread(void *args);

#endif
