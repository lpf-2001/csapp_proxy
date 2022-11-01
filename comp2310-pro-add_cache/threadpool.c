#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include <assert.h>
#include <sched.h>
#include "threadpool.h"
#include "parse.h"
#include "csapp.h"
 
//Stores information of all available thread slots in an array of thread_info
struct thread_info{
    int is_running;
    pthread_t thread_handle;
};

//required for aux_wrapper to detect worker thread termination 
struct _thread_start{
    process_func entry;
    void* args;
};
typedef struct _thread_start thread_start;


typedef struct thread_info threadinfo_t;
threadinfo_t* thread_info_arr;
int global_max_threads = 0;
list* waiting_threads = NULL;

FILE* fp;

sem_t thread_available;
sem_t new_task_incoming;

threadinfo_t aux_thread_handle;

void* aux_thread(void* p);
void pool_init(int max_num)
{
    thread_info_arr = malloc(sizeof(threadinfo_t)*max_num);
    memset(thread_info_arr, 0, sizeof(threadinfo_t)*max_num);
    global_max_threads = max_num;
    waiting_threads = newlist();
    fp = fopen("./test.txt", "w");

    fprintf(fp,"Initialized thread pool: %d\n", max_num);fflush(fp);
    sem_init(&thread_available, 1, max_num);
    sem_init(&new_task_incoming, 1, 0);

    pthread_create(
        &aux_thread_handle,
        NULL,
        aux_thread,
        NULL
    );
}


void* aux_wrapper(void* args)
{
    thread_start* startup_info = args;
    fprintf(fp,"task argument: \n");
    void* result = startup_info->entry(startup_info->args);
    V(&thread_available);
    fprintf(fp,"A task has finished working.\n");fflush(fp);
    return result;
}


void pool_add_worker(process_func process, void*args)
{
    thread_start* start_info = malloc(sizeof(thread_start));
    start_info->args = args;
    start_info->entry = process;

    append(waiting_threads, start_info);

    V(&new_task_incoming);
    fprintf(fp,"Enqueued new task to pool.\n");fflush(fp);
}

void* aux_thread(void* p)
{
    while(1)
    {
        fprintf(fp,"Waiting for new queued task...\n");fflush(fp);
        P(&new_task_incoming);
        fprintf(fp,"Waiting for available thread...\n");fflush(fp);
        P(&thread_available);
        fprintf(fp,"Condition fulfilled, starting task...\n");fflush(fp);

        for(int i=0;i<global_max_threads;i++)
        {
            if(!thread_info_arr[i].is_running) //found an idle thread
            {
                thread_info_arr[i].is_running = 1;
                thread_start* start_info = at(waiting_threads,0)->d;
                list_remove(first(waiting_threads));
                thread_info_arr[i].is_running = 1;
                pthread_create(&thread_info_arr[i].thread_handle,NULL,aux_wrapper, start_info);
                break;
            }
        }
    }
}

void pool_destroy()
{
    fclose(fp);
}