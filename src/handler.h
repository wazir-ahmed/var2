#ifndef _HANDLER_H_INCLUDED
#define _HANDLER_H_INCLUDED

char* make_empty_res(uint8_t ver, uint8_t packt, uint32_t addr);
void send_packet(char* client_info, uv_tcp_t *client, uv_buf_t *send_buf);

void publish(uv_work_t *req);
void fetch(uv_work_t *req);
void subscribe(uv_work_t *req);
void unsubscribe(uv_work_t *req);
void broadcast(uv_work_t *req);

void after_publish(uv_work_t *req, int status);
void after_fetch(uv_work_t *req, int status);
void after_subscribe(uv_work_t *req, int status);
void after_unsubscribe(uv_work_t *req, int status);
void after_broadcast(uv_work_t *req, int status);


#endif