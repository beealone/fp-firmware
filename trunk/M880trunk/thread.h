#ifndef _THREAD_H
#define _THREAD_H
#include <pthread.h>

pthread_mutex_t* mutex_init(void);
int mutexP(pthread_mutex_t *thiz);
int mutexV(pthread_mutex_t *thiz);
void mutex_destroy(pthread_mutex_t *thiz);
pthread_t* thread_create(void *(*func)(void *), void *arg);
int thread_join(pthread_t *thread);
void thread_exit(void);
#endif
