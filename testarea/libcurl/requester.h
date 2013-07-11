#ifndef REQUESTER_H
#define REQUESTER_H

#include <utility>

typedef void(*req_callback_type)(int, int, void*, size_t, void*); // id, error, ptr, size, callbackdata

void req_init();
void req_clean();
int req_load(const char* url, req_callback_type callback, void* callbackdata);
void req_tick();
void req_cancel(int id);
void req_delete(int id);

#endif
