/*
posts.c
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
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "posts.h"
#include <time.h>
#include <stdlib.h>


/*
    opens the provided files and configures the mutex
    this allows fds to be used as one db system
*/
void posts_db_init(struct post_fds *fds, char *meta_path, char *post_path) {
    fds->meta_fd = open(meta_path, O_RDWR|O_APPEND|O_CREAT, 0600);
    fds->post_fd = open(post_path, O_RDWR|O_APPEND|O_CREAT, 0600);
    pthread_mutex_init(&fds->mutex, NULL);
}


/*
    closes the files of fds
*/
void posts_db_close(struct post_fds *fds) {
    close(fds->meta_fd);
    close(fds->post_fd);
}


/*
    gets post number $(num) from fds writing the meta data to $(block)
    and copying the data to a newly mallocd pointer and returning it.
    the data returned is null terminated and MUST be freed
*/
char *get_post(struct post_fds* fds, struct meta_block *block, unsigned long num) {
    get_post_meta(fds, block, num);

    pthread_mutex_lock(&fds->mutex);
    char *ptr = malloc(block->len+1);
    ptr[block->len] = 0; // null terminate
    lseek(fds->post_fd, block->loc, SEEK_SET);
    read(fds->post_fd, ptr, block->len);

    pthread_mutex_unlock(&fds->mutex);
    return ptr;
}


/*
    gets only the metadata of post $(num), copyying it to $(block)
*/
void get_post_meta(struct post_fds* fds, struct meta_block *block, unsigned long num) {
    pthread_mutex_lock(&fds->mutex);
    lseek(fds->meta_fd, sizeof(struct meta_block)*num, SEEK_SET);
    read(fds->meta_fd, block, sizeof(struct meta_block));
    pthread_mutex_unlock(&fds->mutex);
}


/*
    create a new post inserted at the end of fds.
*/
void post(struct post_fds* fds, char *name, char *title, char *data, unsigned long len) {
    pthread_mutex_lock(&fds->mutex);

    struct meta_block block;
    block.loc = lseek(fds->post_fd, 0, SEEK_END);
    block.len = len;
    block.timestamp = time(NULL);
    memcpy(block.title, title, TITLE_LEN);
    memcpy(block.name, name, NAME_LEN);

    write(fds->meta_fd, &block, sizeof(struct meta_block));
    write(fds->post_fd, data, len);

    pthread_mutex_unlock(&fds->mutex);
}


/*
    gets the number of the most recent post
*/
unsigned long post_head(struct post_fds* fds) {
    unsigned long end = lseek(fds->meta_fd, 0, SEEK_END);
    return end / sizeof(struct meta_block) - 1;
}
