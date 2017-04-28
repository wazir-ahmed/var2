#include<stdlib.h>
#include<string.h>
#include<stddef.h>
#include<uv.h>

#include "utils.h"
#include "sockb.h"


v2_clist_t *clist_head = NULL;
v2_clist_t *clist_tail = NULL;

v2_sockb_t* init_sockb(uv_tcp_t *client) {
    v2_sockb_t *node = (v2_sockb_t *) malloc(sizeof(v2_sockb_t));
    if(node == NULL) {
        return NULL;
    }
    
    node->base = NULL;
    node->len = 0;
    node->pb_lock = 0;
    node->pb_head = NULL;
    node->pb_tail = NULL;

    client->data = node;

    return node;
} 

v2_sockb_t* get_sockb(uv_tcp_t* client) {
    return client->data;
}

v2_packb_t* add_packb(v2_sockb_t *sockb, void *src, size_t len) {
    v2_packb_t *node = (v2_packb_t *) malloc(sizeof(v2_packb_t));
    char *dest = (char *)malloc(len); 
    if(node == NULL || src == NULL) {
        return NULL;
    }

    memcpy(dest, src, len);

    node->base = dest;
    node->len = len;
    node->next = NULL;

    if(sockb->pb_head == NULL) {
        sockb->pb_head = node;
        sockb->pb_tail = node;
    }
    else {
        sockb->pb_tail->next = node;
        sockb->pb_tail = node;
    }
    return node;
}

v2_packb_t* get_packb(v2_sockb_t *sockb) {
    return sockb->pb_head;
}

void dequeue_packb(v2_sockb_t *sockb) {
    v2_packb_t *head, *next;
    head = sockb->pb_head;
    if(head != NULL) {
        next = head->next;
        sockb->pb_head = next;
        free(head->base);
        free(head);
    }
    
}

void add_clist(char *key, uv_tcp_t *client) {
    v2_clist_t *node = (v2_clist_t *) malloc(sizeof(v2_clist_t));
    sprintf(node->key, "%s", key);
    node->client = client;
    node->next = NULL;

    if(clist_head == NULL) {
        clist_head = node;
        clist_tail = node;
    }
    else {
        clist_tail->next = node;
        clist_tail = node;
    }
}

uv_tcp_t* get_clist(char *key) {
    v2_clist_t *i;
    i = clist_head;
    
    while(i) {
        if(strcmp(i->key, key) == 0) {
            break;
        } 
        i = i->next;
    }

    if(i != NULL) {
        return i->client;
    }
    else {
        return NULL;
    }
}

