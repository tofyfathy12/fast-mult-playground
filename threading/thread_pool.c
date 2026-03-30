#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include "../utils/Queue.h"

#define THREADS_NUM 20

typedef struct tp {
    pthread_t* threads;
    pthread_mutex_t lock;
    pthread_cond_t signal;
    Queue *job_queue;
    void (*insert_job)(struct tp *pool, Job *job);
    int length;
    bool active;
} thread_pool;

typedef struct j {
    void *(*func)(void *arg);
    void *arg;
} Job;

void *thread_job(thread_pool *pool);
Job job_constructor(void *(*job)(void *arg), void *arg);
thread_pool *construct_pool(int *threads_num);
void insert_job(thread_pool *pool, Job *job);
void destroy_pool(thread_pool *pool);

int main() {

    return 0;
}

Job job_constructor(void *(*func)(void *arg), void *arg) {
    Job job;

    job.func = func;
    job.arg = arg;

    return job;
}

thread_pool *construct_pool(int *threads_num) {
    int num_threads = (threads_num == NULL) ? THREADS_NUM : *threads_num;

    thread_pool* pool = (thread_pool *)malloc(sizeof(thread_pool));

    pool->active = true;
    pool->threads = (pthread_t *)malloc(num_threads * sizeof(pthread_t));
    pool->length = num_threads;

    pool->job_queue = construct_queue();
    pool->lock = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
    pool->signal = (pthread_cond_t)PTHREAD_COND_INITIALIZER;

    pool->insert_job = insert_job;

    for (int i = 0; i < pool->length; i++) {
        pthread_create(pool->threads + i, NULL, thread_job, pool);
    }

    return pool;
}

void insert_job(thread_pool *pool, Job *job) {
    pthread_mutex_lock(&pool->lock);
    pool->job_queue->enqueue(pool->job_queue, job);
    pthread_cond_signal(&pool->signal);
    pthread_mutex_unlock(&pool->lock);
}

void destroy_pool(thread_pool *pool) {

    pthread_mutex_lock(&pool->lock);
    pool->active = false;
    pthread_cond_broadcast(&pool->signal);
    pthread_mutex_unlock(&pool->lock);

    for (int i = 0; i < pool->length; i++) {
        pthread_join(pool->threads[i], NULL);
    }

    free(pool->threads);
    destroy_queue(pool->job_queue);
    free(pool);
}

void *thread_job(thread_pool *pool) {
    while (pool->active) {
        pthread_mutex_lock(&pool->lock);

        while (pool->job_queue->isEmpty(pool->job_queue) && pool->active) {
            pthread_cond_wait(&pool->signal, &pool->lock);
        }

        if (!pool->active) {
            pthread_mutex_unlock(&pool->lock);
            return NULL;
        }
        
        Job *job = (Job *)pool->job_queue->dequeue(pool->job_queue);

        pthread_mutex_unlock(&pool->lock);

        if (job && job->func) {
            job->func(job->arg);
        }
    }
    return NULL;
}