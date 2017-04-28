#include<stdlib.h>
#include<string.h>
#include<uv.h>

#include "var2.h"
#include "utils.h"
#include "cJSON.h"
#include "storage.h"

int add_subscriber(char *client, uint32_t addr) {
    uv_stat_t stat_buf;
    uint64_t file_size;
    uv_buf_t read_data, write_data;
    int res;

    char *path = V2_SUB_LIST_PATH;

    res = fs_stat_sync(event_loop, path, &stat_buf);
    if(res < 0 ) {
        fprintf(stderr, "add_subscriber_stat_:Error %s\n", uv_strerror(res));
        return (-1);
    }
    
    file_size = stat_buf.st_size;
     
    read_data.base = (char *) malloc(file_size);
    read_data.len = file_size;
    
    res = fs_read_sync(event_loop, path, &read_data, O_RDONLY, 0);
    if(res < 0 ) {
        fprintf(stderr, "add_subscriber_read_:Error %s\n", uv_strerror(res));
        return (-1);
    }

    char addr_str[32];
    sprintf(addr_str, "%d", addr);

    cJSON *job_root, *jar_addr, *jit_client, *jit;

    jit_client = cJSON_CreateString(client);
    job_root = cJSON_Parse(read_data.base);
    jar_addr = cJSON_GetObjectItem(job_root, addr_str);
    
    if(jar_addr == NULL) {
        jar_addr = cJSON_CreateArray();
        cJSON_AddItemToObject(job_root, addr_str, jar_addr);
    }

    jit = jar_addr->child;

    while(jit) {
        if(strcmp(jit->valuestring, client) == 0) {
            fprintf(stderr, "add_subscriber_:Error subscriber entry already exist\n"); 
            cJSON_Delete(job_root);
            return (-1);
        }
        jit = jit->next;
    }

    cJSON_AddItemToArray(jar_addr, jit_client);

    write_data.base = cJSON_Print(job_root);
    write_data.len = strlen(write_data.base);
    cJSON_Delete(job_root);

    res = fs_write_sync(event_loop, path, &write_data, O_WRONLY | O_TRUNC, 0);
    if(res < 0 ) {
        fprintf(stderr, "add_subscriber_write_:Error %s\n", uv_strerror(res));
        return (-1);
    }

    return 0;
}

int remove_subscriber(char *client, uint32_t addr) {
    uv_stat_t stat_buf;
    uint64_t file_size;
    uv_buf_t read_data, write_data;
    int res;

    char *path = V2_SUB_LIST_PATH;

    res = fs_stat_sync(event_loop, path, &stat_buf);
    if(res < 0 ) {
        fprintf(stderr, "remove_subscriber_stat_:Error %s\n", uv_strerror(res));
        return (-1);
    }
    
    file_size = stat_buf.st_size;
     
    read_data.base = (char *) malloc(file_size);
    read_data.len = file_size;
    
    res = fs_read_sync(event_loop, path, &read_data, O_RDONLY, 0);
    if(res < 0 ) {
        fprintf(stderr, "remove_subscriber_read_:Error %s\n", uv_strerror(res));
        return (-1);
    }

    char addr_str[32];
    int i;
    sprintf(addr_str, "%d", addr);

    cJSON *job_root, *jar_addr, *jit;

    job_root = cJSON_Parse(read_data.base);
    jar_addr = cJSON_GetObjectItem(job_root, addr_str);
    
    if(jar_addr != NULL) {
        jit = jar_addr->child;    
    }
    else {
        jit = NULL;
    }

    for(i = 0; jit != NULL; jit = jit->next) {
        if(strcmp(jit->valuestring, client) == 0) {
            cJSON_DeleteItemFromArray(jar_addr, i);
            break;
        }
    }  

    if(jit == NULL) {
        cJSON_Delete(job_root);
        return (-1);
    }


    write_data.base = cJSON_Print(job_root);
    write_data.len = strlen(write_data.base);

    cJSON_Delete(job_root);

    res = fs_write_sync(event_loop, path, &write_data, O_WRONLY | O_TRUNC, 0);
    if(res < 0 ) {
        fprintf(stderr, "add_subscriber_write_:Error %s\n", uv_strerror(res));
        return (-1);
    }

    return 0;
}

int get_subscriber(uint32_t addr, uv_buf_t* buf) {
    int res, len;
    char addr_str[32], *str, **str_arr;
    char *path = V2_SUB_LIST_PATH;
    uv_stat_t stat_buf;
    uint64_t file_size;
    uv_buf_t read_buf;

    res = fs_stat_sync(event_loop, path, &stat_buf);
    if(res < 0 ) {
        fprintf(stderr, "get_subscriber_stat_:Error %s\n", uv_strerror(res));
        return (-1);
    }
    
    file_size = stat_buf.st_size;
     
    read_buf.base = (char *) malloc(file_size);
    read_buf.len = file_size;
    
    res = fs_read_sync(event_loop, path, &read_buf, O_RDONLY, 0);
    if(res < 0 ) {
        fprintf(stderr, "get_subscriber_read_:Error %s\n", uv_strerror(res));
        return (-1);
    }

    
    sprintf(addr_str, "%d", addr);

    cJSON *job_root, *jar_addr, *jit;

    job_root = cJSON_Parse(read_buf.base);
    jar_addr = cJSON_GetObjectItem(job_root, addr_str);
    if(jar_addr != NULL) {
        len = cJSON_GetArraySize(jar_addr);
    }
    else {
        len = 0;
    }

    buf->len = len;

    if(len == 0) {
        cJSON_Delete(job_root);
        buf->base = NULL;
        return 0;
    }

    str_arr = (char **) malloc(len * sizeof(char *));

    jit = jar_addr->child;
    len = len - 1;
    while(jit != NULL) {
        str = (char *) malloc(V2_SOCK_STR_LEN);
        sprintf(str, "%s", jit->valuestring);
        str_arr[len] = str;

        jit = jit->next;
        len--;
    }

    buf->base = (void *) str_arr;
    cJSON_Delete(job_root);
    return 0;
    
}
