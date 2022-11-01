#ifndef THREADPOOL_H
#define THREADPOOL_H

typedef void* (*process_func)(void*);

void pool_init(int max_thread_num);
void pool_add_worker(process_func process, void *arg);
void pool_destroy();

#endif