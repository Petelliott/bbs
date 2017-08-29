/*
login.h
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
#ifndef _LOGIN_H
#define _LOGIN_H

#include <gdbm.h>
#include <pthread.h>

struct logindb {
    GDBM_FILE db;
    pthread_mutex_t mutex;
};

void logindb_init(struct logindb* db, char *df_file);

int add_login(struct logindb* db, char *user, char *password);
int login(struct logindb* db, char *user, char *password);

#endif
