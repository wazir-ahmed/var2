#ifndef _STORAGE_H_INCLUDED
#define _STORAGE_H_INCLUDED

#define V2_SUB_LIST_PATH "storage/sub-list.json"
#define V2_SUB_LIST_ADDR 4294967295u

int add_subscriber(char *client, uint32_t addr);
int remove_subscriber(char *client, uint32_t addr);
int get_subscriber(uint32_t addr, uv_buf_t* buf);

#endif
