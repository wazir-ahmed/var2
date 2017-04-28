#ifndef _SOCKB_H_INCLUDED
#define _SOCKB_H_INCLUDED

#define SOCK_KEY_LEN 48

typedef struct v2_packb {
    char *base;
    size_t len;
    struct v2_packb *next;
} v2_packb_t;

typedef struct v2_sockb {
    char *base;
    size_t len;
    v2_packb_t *pb_head;
    v2_packb_t *pb_tail;
    int pb_lock;
} v2_sockb_t;

typedef struct client_list {
    char key[SOCK_KEY_LEN];
    uv_tcp_t *client;
    struct client_list *next;
} v2_clist_t;

extern v2_clist_t *clist_head;
extern v2_clist_t *clist_tail;

v2_sockb_t* init_sockb(uv_tcp_t*);
v2_sockb_t* get_sockb(uv_tcp_t*);

v2_packb_t* add_packb(v2_sockb_t*, void*, size_t);
v2_packb_t* get_packb(v2_sockb_t*);
void dequeue_packb(v2_sockb_t*);

void add_clist(char*, uv_tcp_t*);
uv_tcp_t* get_clist(char*);

#endif
