#include<sys/socket.h>
#include<stdint.h>
#include<stdlib.h>
#include<string.h>
#include<uv.h>

#include "var2.h"
#include "sockb.h"
#include "utils.h"

v2_rwlock_t *rwlock_head;

char* get_client_info(uv_tcp_t* client, char* dest) {
    struct sockaddr_storage *addr = (struct sockaddr_storage*) malloc(sizeof(struct sockaddr_storage));
    int addr_len = sizeof(struct sockaddr_storage);
    int ip_port;
    char ip_addr_str[INET6_ADDRSTRLEN];

    uv_tcp_getpeername(client, (struct sockaddr *)addr, &addr_len);
    
    if (addr->ss_family == AF_INET) {
        uv_ip4_name((struct sockaddr_in*) addr, ip_addr_str, INET_ADDRSTRLEN);
        ip_port =  ((struct sockaddr_in*)addr)->sin_port;
    }
    else {
        uv_ip6_name((struct sockaddr_in6*) addr, ip_addr_str, INET6_ADDRSTRLEN);
        ip_port =  ((struct sockaddr_in6*)addr)->sin6_port;
    }
    ip_port = ntohs(ip_port);

    sprintf(dest, "%s : %d", ip_addr_str, ip_port);

    return dest;
}

void print_buf(char *base, size_t len) {
    int i;
    printf("\\start    ");
    for(i = 0; i < len; i++)
        printf("%d    ", (uint8_t) base[i]);
    printf("\\end\n");
}

uint32_t addr_from_path(char *path) {
    char *end;
    uint32_t addr;
    path = path + V2_ST_ROOT_PATH_LEN;
    addr = (uint32_t)strtoul(path, &end, 10);
    return addr;
}

char* strncpy_safe(char *des, char *src, size_t n) {
    strncpy(des, src, n - 1);
    des[n-1] = '\0';
    return des;
}

int fs_read_sync(uv_loop_t* loop, char *path, uv_buf_t *buffer, int flags, int mode) {
    int res;
    uv_fs_t open_req, read_req, close_req;
    
    res = uv_fs_open(loop, &open_req, path, flags, mode, NULL);
    if(res < 0 ) {
        return res;
    }

    res = uv_fs_read(loop, &read_req, open_req.result, buffer, 1, 0, NULL);
    if(res < 0 ) {
        return res;
    }
    
    uv_fs_close(loop, &close_req, open_req.result, NULL);
    return res;
}

int fs_write_sync(uv_loop_t* loop, char *path, uv_buf_t *buffer, int flags, int mode) {
    int res;
    uv_fs_t open_req, write_req, close_req;

    res = uv_fs_open(loop, &open_req, path, flags, mode, NULL);
    if(res < 0 ) {
        return res;
    }
    
    res = uv_fs_write(loop, &write_req, open_req.result, buffer, 1, 0, NULL);
    if(res < 0 ) {
        return res;
    }
    
    uv_fs_close(loop, &close_req, open_req.result, NULL);
    return res;
}

int fs_stat_sync(uv_loop_t* loop, char *path, uv_stat_t *stat) {
    int res;
    uv_fs_t stat_req;

    res = uv_fs_stat(loop, &stat_req, path, NULL);
    if(res < 0) {
        return res;
    }
    memcpy(stat, &(stat_req.statbuf), sizeof(stat_req.statbuf));
    return res;
}

uv_rwlock_t* add_rwlock(uint32_t key) {
    v2_rwlock_t *node;
    node = (v2_rwlock_t *) malloc(sizeof(v2_rwlock_t));
    node->key = key;
    uv_rwlock_init(&(node->lock));

    if(rwlock_head == NULL) {
        node->next = NULL;
        node->prev = NULL;
    }
    else {
        rwlock_head->prev = node;
        node->next = rwlock_head;
        node->prev = NULL;
    }

    rwlock_head = node;
    return &(rwlock_head->lock);
}

uv_rwlock_t* get_rwlock(uint32_t key) {
    v2_rwlock_t *i = rwlock_head;

    while(i && i->key != key) {
        i = i->next;
    }

    if(i == NULL) {
        return add_rwlock(key);
    }
    else {
        return &(i->lock);
    }
}

void rd_lock(uint32_t key) {
    uv_rwlock_t *lock = get_rwlock(key);
    uv_rwlock_rdlock(lock);
    printf("\nread lock acquired on key : %u\n", key);  
}

void wr_lock(uint32_t key) {
    uv_rwlock_t *lock = get_rwlock(key);
    uv_rwlock_wrlock(lock);
    printf("\nwrite lock acquired on key : %u\n", key);
}

void rd_unlock(uint32_t key) {
    uv_rwlock_t *lock = get_rwlock(key);
    uv_rwlock_rdunlock(lock);
    printf("\nunlocked read lock acquired on key : %u\n", key);
}

void wr_unlock(uint32_t key) {
    uv_rwlock_t *lock = get_rwlock(key);
    uv_rwlock_wrunlock(lock);
    printf("\nunlocked write lock acquired on key : %u\n", key);
}


