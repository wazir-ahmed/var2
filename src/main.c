#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#include<string.h>
#include<sys/types.h>
#include<unistd.h>
#include<uv.h>

#include "utils.h"
#include "sockb.h"
#include "parser.h"
#include "handler.h"
#include "var2.h"

#define HOST "127.0.0.1"
#define PORT 1121
#define BACKLOG 128

#define V2_MAXTHREAD "10"

uv_loop_t *event_loop;

void pack_ana(uv_tcp_t*, v2_sockb_t*);

void pack_ana_itr(uv_work_t *req, int status) {
    struct pack_baton *baton;
    uv_tcp_t *client;
    v2_sockb_t *sockb;
    v2_packb_t *pb_head;

    baton = (struct pack_baton *) req->data;
    client = baton->client;
    sockb = get_sockb(client);
    
    sockb->pb_lock = 0;
    dequeue_packb(sockb);
    pb_head = sockb->pb_head;

    if(pb_head != NULL) {
        pack_ana(client, sockb);
    }

    free(baton);
}

void pack_ana(uv_tcp_t *client, v2_sockb_t *sock_buf) {
    char client_info[V2_SOCK_STR_LEN];
    char *base;
    size_t len;
    uint8_t packt_byte;
    uint8_t packt;
    v2_packb_t* pb_head;
    struct pack_baton *baton;

    if(sock_buf->pb_lock != 0) {
        return;
    }

    sock_buf->pb_lock = 1;
    pb_head = sock_buf->pb_head;
    base = pb_head->base;
    len = pb_head->len;
   
    printf("\n-----------------\n");
    printf("NEW PACKET from %s \n", get_client_info(client, client_info));
    print_buf(base, len);
    
    packt_byte = (uint8_t)base[0];
    packt = parse_packet_t(packt_byte);

    baton = (struct pack_baton*)malloc(sizeof(struct pack_baton));
    baton->req.data = (void *) baton;
    
    baton->base = base;
    baton->len = len;
    baton->client = client;

    if(packt == V2_PACKT_PUBLISH) {
        uv_queue_work(event_loop, &baton->req, publish, after_publish);    
    }
    else if(packt == V2_PACKT_FETCH) {
        uv_queue_work(event_loop, &baton->req, fetch, pack_ana_itr);
    }
    else if(packt == V2_PACKT_SUB) {
        uv_queue_work(event_loop, &baton->req, subscribe, pack_ana_itr);
    }
    else if(packt == V2_PACKT_UNSUB) {
        uv_queue_work(event_loop, &baton->req, unsubscribe, pack_ana_itr);
    }
}

v2_sockb_t* packetify(uv_tcp_t* client, uv_buf_t* data) {
    char *base;
    size_t len;
    size_t pack_len;
    uv_buf_t buffer;
    v2_sockb_t *sock_buf;

    uint32_t payload_len;
    uint8_t contf_byte;
    uint8_t cont_flag;

    sock_buf = get_sockb(client);

    if(sock_buf == NULL) {
        sock_buf = init_sockb(client);    
    }

    if(data != NULL) {
        len = sock_buf->len + data->len;
        base = (char *) malloc(len);
        memcpy(base, sock_buf->base, sock_buf->len);
        memcpy(base + sock_buf->len, data->base, data->len);
    }

    free(sock_buf->base);
    free(data->base);

    buffer.base = base;
    buffer.len = len;

    while(buffer.len >= V2_PACKHEAD_MINSIZE) {
        contf_byte = buffer.base[V2_PACKBYTE_CONTF];
        cont_flag = parse_cont_flag(contf_byte);
        if(cont_flag == 0) {
            pack_len = 5;
            add_packb(sock_buf, buffer.base, pack_len);
            buffer.base += pack_len;
            buffer.len -= pack_len;
        }
        else {
            payload_len = parse_payload_len((uint8_t *)(buffer.base + V2_PACKBYTE_PAYLOADLEN), V2_PACKFIELDLEN_PAYLOADLEN);
            pack_len = V2_PACKHEAD_MAXSIZE + payload_len;
            if(buffer.len < pack_len) {
                break;
            } 
            else {
                add_packb(sock_buf, buffer.base, pack_len);
                buffer.base += pack_len;
                buffer.len -= pack_len;
            }     
        }
    }

    if(buffer.len != 0) {
        sock_buf->base = (char *) malloc(buffer.len);
        memcpy(sock_buf->base, buffer.base, buffer.len);
    }
    else {
        sock_buf->base = NULL;
    }
    sock_buf->len = buffer.len;
    free(base);
    
    return sock_buf;
}

void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
    buf->base = (char*) malloc(suggested_size);
    buf->len = suggested_size;
}

void on_data(uv_stream_t* client, ssize_t nread, const uv_buf_t* uv_buf) {
    if(nread < 0) {
        if(nread != UV_EOF)
            fprintf(stderr, "on_data_:Error %s\n", uv_strerror(nread));
        uv_close((uv_handle_t*) client, NULL);
        free(uv_buf->base);
        return;
    }
    else if(nread == 0) {
        free(uv_buf->base);
        return;
    }
    
    uv_buf_t data = uv_buf_init(uv_buf->base, nread);
    v2_sockb_t *sock_buf = packetify((uv_tcp_t*) client, &data);
    if(sock_buf->pb_lock == 0 && sock_buf->pb_head != NULL) {
        pack_ana((uv_tcp_t *) client, sock_buf);
    }
}

void on_new_connection(uv_stream_t* server, int status) {
    char client_info[V2_SOCK_STR_LEN];

    if(status < 0) {
        fprintf(stderr, "on_new_connection_:Error %s\n", uv_strerror(status));
        return;
    }

    uv_tcp_t *client = (uv_tcp_t*) malloc(sizeof(uv_tcp_t));
    uv_tcp_init(event_loop, client);
    
    if (uv_accept(server, (uv_stream_t*) client) == 0) {

        get_client_info(client, client_info);

        add_clist(client_info, client);
        
        printf("\n-----------------\n");
        printf("Connected to %s\n", client_info);
        
        client->data = NULL;
        uv_read_start((uv_stream_t*) client, alloc_buffer, on_data);
    }
    else {
        uv_close((uv_handle_t*) client, NULL);
    }
}

int main() {
    int res;
    struct sockaddr_in addr;
    uv_tcp_t server;

    pid_t id = getpid();
    printf("Process ID : %d\n", (int)id);

    rwlock_head = NULL;

    uv_os_setenv("UV_THREADPOOL_SIZE", V2_MAXTHREAD);   
    

    event_loop = uv_default_loop();

    res = uv_tcp_init(event_loop, &server);
    assert(res == 0);

    res = uv_ip4_addr(HOST, PORT, &addr);
    assert(res == 0);

    res = uv_tcp_bind(&server, (struct sockaddr*) &addr, 0);
    assert(res == 0);

    res = uv_listen((uv_stream_t*) &server, BACKLOG, on_new_connection);
    assert(res == 0);
    printf("Server started at %s : %d\n", HOST, PORT);

    return uv_run(event_loop, UV_RUN_DEFAULT);
}