/*
posts.h
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
#ifndef _POSTS_H
#define _POSTS_H

#define TITLE_LEN 32
#define NAME_LEN 24

struct post_fds {
    int meta_fd;
    int post_fd;
    pthread_mutex_t mutex;
};

struct meta_block {
    unsigned long loc;
    unsigned long len;
    time_t timestamp;
    char title[TITLE_LEN];
    char name[NAME_LEN];
};

void posts_db_init(struct post_fds *fds, char *meta_path, char *post_path);
void posts_db_close(struct post_fds *fds);

char *get_post(struct post_fds* fds, struct meta_block *block, unsigned long num);
void get_post_meta(struct post_fds* fds, struct meta_block *block, unsigned long num);

void post(struct post_fds* fds, char *name, char *title, char *data, unsigned long len);

#endif
