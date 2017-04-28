#ifndef _UTILS_H_INCLUDED
#define _UTILS_H_INCLUDED

#define V2_LOCK_RD 1
#define V2_LOCK_WR 2
#define V2_UNLOCK_RD 3
#define V2_UNLOCK_WR 4

#define V2_LOCK_ENABLE 1
#define V2_LOCK_DISABLE 2

typedef struct v2_rwlock {
    uint32_t key;
    uv_rwlock_t lock;
    struct v2_rwlock *next;
    struct v2_rwlock *prev;
} v2_rwlock_t;

extern v2_rwlock_t *rwlock_head;

char* get_client_info(uv_tcp_t*, char*);
void print_buf(char*, size_t);
char* strncpy_safe(char*, char*, size_t);
uint32_t add_from_path(char*);

int fs_read_sync(uv_loop_t*, char*, uv_buf_t*, int, int);
int fs_write_sync(uv_loop_t*, char*, uv_buf_t*, int, int);
int fs_stat_sync(uv_loop_t*, char*, uv_stat_t*);

uv_rwlock_t* get_rwlock(uint32_t);
uv_rwlock_t* add_rwlock(uint32_t);

void rd_lock(uint32_t key);
void wr_lock(uint32_t key);
void rd_unlock(uint32_t key);
void wr_unlock(uint32_t key);

#endif