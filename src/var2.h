#ifndef _VAR2_H_INCLUDED
#define _VAR2_H_INCLUDED

#define V2_PACKBYTE_VERSION 0
#define V2_PACKBYTE_PACKT 0
#define V2_PACKBYTE_ADDR 1
#define V2_PACKBYTE_CONTF 4
#define V2_PACKBYTE_PAYLOADLEN 5
#define V2_PACKBYTE_CONTENT 9
#define V2_PACKBYTE_ESIZE 10
#define V2_PACKBYTE_PAYLOAD 12

#define V2_PACKFIELDLEN_ADDR 4
#define V2_PACKFIELDLEN_PAYLOADLEN 4
#define V2_PACKFIELDLEN_ESIZE 2

#define V2_PACKHEAD_MINSIZE 5
#define V2_PACKHEAD_MAXSIZE 12
#define V2_PACK_EMPTYRES_SIZE 12

#define V2_PACKT_PUBLISH 4
#define V2_PACKT_FETCH 5
#define V2_PACKT_SUB 6
#define V2_PACKT_UNSUB 7
#define V2_PACKT_BROAD 8
#define V2_PACKT_BUFFER 0
#define V2_PACKT_BUFFET 0

#define V2_ST_ROOT_PATH "storage/"
#define V2_ST_ROOT_PATH_LEN 8
#define V2_SOCK_STR_LEN 48

struct pack_baton {
    char *base;
    size_t len;
    uv_tcp_t *client;
    uv_work_t req;
};

extern uv_loop_t *event_loop;

void pack_ana_itr(uv_work_t*, int);

#endif
