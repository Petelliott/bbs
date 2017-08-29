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
#include "login.h"
#include <string.h>
#include <stddef.h>
#define __USE_GNU
#include <crypt.h>
#include <stdlib.h>


/*
    converts a char into one of the chars specifyed by crypt(3) for salts
*/
char remap_char(char num) {
    num %= 64;
    if (num < 26) {
        num += 'a';
    } else if (num < 52) {
        num += 'A';
    } else if (num < 62) {
        num += '0';
    } else if (num == 62) {
        num = '.';
    } else if (num == 63) {
        num = '/';
    }
    return num;
}


/*
    generates a random crypt(3) salt
*/
void gen_salt(char *salt) {
    memcpy(salt, "$5$", 3);
    for (size_t i = 3; i < 19; ++i) {
        salt[i] = remap_char(rand());
    }
    salt[19] = '$';
    salt[20] = '\0';
}


/*
    initializes gdbm and the mutex together.
*/
void logindb_init(struct logindb* db, char *df_file) {
    db->db = gdbm_open(df_file, 0, GDBM_WRCREAT, 0600, NULL);
    pthread_mutex_init(&db->mutex, NULL);
}


/*
    add a user to the logindb.
    returns 1 if successful 0 if not
*/
int add_login(struct logindb* db, char *user, char *password) {
    struct crypt_data data;
    data.initialized = 0;

    char salt[21];
    gen_salt(salt);

    char *pass_crypt = crypt_r(password, salt, &data);

    pthread_mutex_lock(&db->mutex);
    datum user_d = { .dptr = user, .dsize = strlen(user) };
    datum pass_d = { .dptr = pass_crypt, .dsize = strlen(pass_crypt)+1 };

    int status = !gdbm_store(db->db, user_d, pass_d, GDBM_INSERT);

    pthread_mutex_unlock(&db->mutex);
    return status;
}


/*
    validate a users login credentials.
    returns 1 if valid, 0 if not
*/
int login(struct logindb* db, char *user, char *password) {

    pthread_mutex_lock(&db->mutex);
    datum user_d = { .dptr = user, .dsize = strlen(user) };
    datum pass_d = gdbm_fetch(db->db, user_d);
    pthread_mutex_unlock(&db->mutex);

    if (pass_d.dptr == NULL) {
        return 0;
    }

    struct crypt_data data;
    data.initialized = 0;
    char *pass_crypt = crypt_r(password, pass_d.dptr, &data);

    int status = (strncmp(pass_crypt, pass_d.dptr, 64) == 0);

    free(pass_d.dptr);
    return status;
}
