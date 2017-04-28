#include<stdlib.h>
#include<stdint.h>
#include<string.h>
#include<time.h>
#include<uv.h>

#include "utils.h"
#include "var2.h"
#include "parser.h"
#include "sockb.h"  
#include "storage.h"
#include "handler.h"

char* make_empty_res(uint8_t ver, uint8_t packt, uint32_t addr) {
    char *pack = (char *) malloc(V2_PACK_EMPTYRES_SIZE);
    memset(pack, 0, V2_PACK_EMPTYRES_SIZE);

    pack[0] = (ver << 4) | packt;
   
    addr = addr <<1 | 0b00000001; 
    pack[4] = (uint8_t) addr;
    addr = addr >> 8;
    pack[3] = (uint8_t) addr;
    addr = addr >> 8;
    pack[2] = (uint8_t) addr;
    addr = addr >> 8;
    pack[1] = (uint8_t) addr;

    return pack;
}

void send_packet(char *client_info, uv_tcp_t *client, uv_buf_t *send_buf) {
    int res;
    uv_write_t *write_req;

    write_req = (uv_write_t *) malloc(sizeof(uv_write_t));
    res = uv_write(write_req, (uv_stream_t *)client, send_buf, 1, NULL);
    if(res < 0 ) {
        fprintf(stderr, "send_packet_:Error %s\n", uv_strerror(res));
        return;
    }

    printf("\n-----------------\n");
    printf("SENT A PACKET to %s \n", client_info);
    print_buf(send_buf->base, send_buf->len);
    
    //free(write_req);
}

void publish(uv_work_t *req) {
    int res;
    uint32_t addr;

    char* path;
    int path_len;
    uv_buf_t data;
    struct pack_baton *baton;

    baton = (struct pack_baton *) req->data;
    data.base = baton->base;
    data.len = baton->len;
    
    addr = parse_address((uint8_t *) data.base + V2_PACKBYTE_ADDR, V2_PACKFIELDLEN_ADDR);

    path_len = strlen(V2_ST_ROOT_PATH) + (V2_PACKFIELDLEN_ADDR * 8) + 1;
    path = (char *) malloc(path_len);
    sprintf(path, "%s%d", V2_ST_ROOT_PATH, addr);

    wr_lock(addr);
    res = fs_write_sync(event_loop, path, &data, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
    wr_unlock(addr);
    
    if(res < 0 ) {
        fprintf(stderr, "publish_write_:Error %s\n", uv_strerror(res));
        return;
    }

    printf("\n-----------------\n");
    printf("STORED A PACKET\n");
    print_buf(data.base, data.len);
}

void fetch(uv_work_t *req) {
    int res;
    char client_info[V2_SOCK_STR_LEN];
    uv_stat_t stat_buf;
    uv_buf_t send_buf;
    
    char *base;
    uv_tcp_t *client;
    struct pack_baton *baton;
    
    uint8_t ver, packt;
    uint32_t addr;

    char* path;
    int path_len;
    uint64_t file_size;

    baton = (struct pack_baton *) req->data;
    client = baton->client;
    base = baton->base;
    
    ver = parse_version((uint8_t)base[V2_PACKBYTE_VERSION]);
    packt = parse_packet_t((uint8_t)base[V2_PACKBYTE_PACKT]);
    addr = parse_address((uint8_t *)base + V2_PACKBYTE_ADDR, V2_PACKFIELDLEN_ADDR);

    path_len = strlen(V2_ST_ROOT_PATH) + (V2_PACKFIELDLEN_ADDR * 8) + 1;
    path = (char *) malloc(path_len);
    sprintf(path, "%s%d", V2_ST_ROOT_PATH, addr);

    res = fs_stat_sync(event_loop, path, &stat_buf);
    if(res == UV_ENOENT) {
        file_size = 0;
    }
    else if(res < 0 ) {
        fprintf(stderr, "fetch_stat_:Error %s\n", uv_strerror(res));
        return;
    }
    else {
        file_size = stat_buf.st_size;
    }
     
    send_buf.base = (char *) malloc(file_size);
    send_buf.len = file_size;
    
    if(file_size != 0) {
        rd_lock(addr);
        res = fs_read_sync(event_loop, path, &send_buf, O_CREAT | O_RDONLY, S_IRUSR | S_IWUSR);
        rd_unlock(addr);
        if(res < 0 ) {
            fprintf(stderr, "fetch_read_:Error %s\n", uv_strerror(res));
            return;
        }
        send_buf.base[V2_PACKBYTE_PACKT] += 1;
    }
    else {
        send_buf.base = make_empty_res(ver, packt, addr);
        send_buf.len = V2_PACK_EMPTYRES_SIZE;
    }
    
    get_client_info(client, client_info);
    send_packet(client_info, client, &send_buf);

    free(send_buf.base);
}

void subscribe(uv_work_t *req) {
    int res;
    char *base;
    uint32_t addr, sub_list_addr;
    uv_tcp_t *client;
    char client_info[V2_SOCK_STR_LEN];
    struct pack_baton *baton;
    
    baton = (struct pack_baton *) req->data;
    client = baton->client;
    base = baton->base;

    addr = parse_address((uint8_t *) base + V2_PACKBYTE_ADDR, V2_PACKFIELDLEN_ADDR);
    get_client_info(client, client_info);

    sub_list_addr = (uint32_t) V2_SUB_LIST_ADDR; 
    wr_lock(sub_list_addr);
    res = add_subscriber(client_info, addr);
    wr_unlock(sub_list_addr);

    if(res == 0) {
        printf("\n-----------------\n");
        printf("ADDED SUBSCRIBER %s to %d\n", client_info, addr);
    }
    else {
        fprintf(stderr, "failed to add subscriber %s to %d\n", client_info, addr);
    }
}

void unsubscribe(uv_work_t *req) {
    int res;
    char *base;
    uint32_t addr, sub_list_addr;
    uv_tcp_t *client;
    char client_info[V2_SOCK_STR_LEN];
    struct pack_baton *baton;
    
    baton = (struct pack_baton *) req->data;
    client = baton->client;
    base = baton->base;

    addr = parse_address((uint8_t *) base + V2_PACKBYTE_ADDR, V2_PACKFIELDLEN_ADDR);
    get_client_info(client, client_info);

    sub_list_addr = (uint32_t) V2_SUB_LIST_ADDR; 
    wr_lock(sub_list_addr);
    res = remove_subscriber(client_info, addr);
    wr_unlock(sub_list_addr);

    if(res == 0) {
        printf("\n-----------------\n");
        printf("REMOVED SUBSCRIBER %s from %d\n", client_info, addr);
    }
    else {
        fprintf(stderr, "failed to remove subscriber %s to %d\n", client_info, addr);
    }
}

void broadcast(uv_work_t *req) {
    int res, len;
    char *pack, **str_arr, pub_client_info[V2_SOCK_STR_LEN];
    size_t pack_len;
    uint32_t addr, sub_list_addr;
    uv_buf_t list, send_buf;
    uv_tcp_t *pub_client, *sub_client; 
    struct pack_baton *baton;

    sub_list_addr = (uint32_t) V2_SUB_LIST_ADDR;

    baton = (struct pack_baton *) req->data;
    pack = baton->base;
    pack_len = baton->len; 
    pub_client = baton->client;

    addr = parse_address((uint8_t *)pack + V2_PACKBYTE_ADDR, V2_PACKFIELDLEN_ADDR);

    pack[V2_PACKBYTE_PACKT] = change_packt(V2_PACKT_BROAD, (uint8_t) pack[V2_PACKBYTE_PACKT]);

    send_buf.base = pack;
    send_buf.len = pack_len;
 
    wr_lock(sub_list_addr);
    res = get_subscriber(addr, &list);
    wr_unlock(sub_list_addr);

    get_client_info(pub_client, pub_client_info);
    
    if(res == 0 && list.base != NULL) {
        len = list.len - 1;
        str_arr = (char **) list.base;
        for(; len >= 0; len-- ) {
            sub_client = get_clist(str_arr[len]);
            if(sub_client != NULL && strcmp(str_arr[len], pub_client_info) != 0) {
                send_packet(str_arr[len], sub_client, &send_buf);
            }
        }
    }
}

void after_publish(uv_work_t *req, int status) {
    struct pack_baton *baton = (struct pack_baton *) req->data;
    uv_tcp_t *client = baton->client;
    v2_sockb_t *sockb = (v2_sockb_t *)client->data;
    v2_packb_t *pb_head = sockb->pb_head;

    char *base = pb_head->base;
    size_t len = pb_head->len;

    char *pack = (char *) malloc(len);
    size_t pack_len = len;

    memcpy(pack, base, len);

    struct pack_baton *new_baton = (struct pack_baton *) malloc(sizeof(struct pack_baton));
    new_baton->base = pack;
    new_baton->len = pack_len;
    new_baton->client = client;
    new_baton->req.data = (void *) new_baton;

    uv_queue_work(event_loop, &new_baton->req, broadcast, after_broadcast);
    pack_ana_itr(req, status);
}

void after_fetch(uv_work_t *req, int status) {
    // freereq->data(baton)->base;
    //close req;
}

void after_subscribe(uv_work_t *req, int status) {
    //code here
}

void after_unsubscribe(uv_work_t *req, int status) {
    //code here
}

void after_broadcast(uv_work_t *req, int status) {
    //code here
}