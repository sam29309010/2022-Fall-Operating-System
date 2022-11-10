#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <pthread.h>

#define MAXTHREAD (8-1)
pthread_barrier_t barrier;
int num_thread;
double period_busy;

typedef struct {
    pthread_t thread_id;
    int num_thread; // thread id to be printed out
    int sched_policy;
    int sched_priority;
} thread_info_t;


void *thread_func(void *arg);

int main(int argc, char *argv[])
{
    thread_info_t thread_infos[MAXTHREAD];
    // pthread_t thread_handles[MAXTHREAD];

    /* 1. Parse program arguments */
    int opt, i;
    char *token;
    while ((opt = getopt(argc, argv, "n:t:s:p:")) != -1)
    {
        switch (opt)
        {
        case 'n':
            num_thread = atoi(optarg);
            break;

        case 't':
            period_busy = atof(optarg);
            break;

        case 's':
            i = 0;
            token = strtok(optarg, ",");
            while (token != NULL)
            {
                thread_infos[i++].sched_policy = (strcmp(token, "NORMAL") == 0) ? SCHED_OTHER : SCHED_FIFO;
                token = strtok(NULL, ",");
            }
            break;

        case 'p':
            i = 0;
            token = strtok(optarg, ",");
            while (token != NULL)
            {
                thread_infos[i++].sched_priority = atoi(token);
                token = strtok(NULL, ",");
            }
            break;
        default:
            break;
        }
    }

    pthread_attr_t attrs[MAXTHREAD];
    sched_param params[MAXTHREAD];
    for (int i = 0; i < num_thread; i++) {
        /* 4. Set the attributes to each thread */
        pthread_attr_init(&attrs[i]);
        // pthread_attr_setschedpolicy(&attrs[i], thread_infos[i].sched_policy);
        // params[i].sched_priority = (thread_infos[i].sched_priority >= 0) ? thread_infos[i].sched_priority : 0;
        // pthread_attr_setschedparam(&attrs[i], &params[i]);
    }

    /* 2. Create <num_threads> worker threads */
    pthread_barrier_init(&barrier, NULL, num_thread + 1);
    for (int i = 0; i < num_thread; i++)
    {
        thread_infos[i].num_thread = i;
        pthread_create(&thread_infos[i].thread_id, &attrs[i], thread_func, (void *) &thread_infos[i].num_thread);
    }

    /* 3. Set CPU affinity */
    // set host thread affine to core 0
    cpu_set_t cpuset_zero;
    pthread_t thread = pthread_self();
    CPU_ZERO(&cpuset_zero);
    CPU_SET(0, &cpuset_zero);
    pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset_zero);

    for (int i = 0; i < num_thread; i++) {
        /* 4. Set the attributes to each thread */
        // pthread_attr_init(&attrs[i]);
        params[i].sched_priority = (thread_infos[i].sched_priority >= 0) ? thread_infos[i].sched_priority : 0;
        pthread_setschedparam(thread_infos[i].thread_id, thread_infos[i].sched_policy, &params[i]);        
    }

    /* 5. Start all threads at once */
    pthread_barrier_wait(&barrier);

    /* 6. Wait for all threads to finish  */
    for (int i = 0; i < num_thread; i++)
        pthread_join(thread_infos[i].thread_id, NULL);
    
}

void *thread_func(void *arg)
{
    cpu_set_t cpuset_one;
    pthread_t thread = pthread_self();
    CPU_ZERO(&cpuset_one);
    CPU_SET(1, &cpuset_one);
    pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset_one);

    pthread_getaffinity_np(thread, sizeof(cpu_set_t), &cpuset_one);
    // printf("Set returned by pthread_getaffinity_np() contained:\n");
    // for (int j = 0; j < CPU_SETSIZE; j++)
    //     if (CPU_ISSET(j, &cpuset_one))
    //         printf("    CPU %d\n", j);


    // DEBUG
    // pthread_t tid = pthread_self();
    // sched_param param;
    // int priority;
    // int policy;
    // pthread_getschedparam(tid, &policy, &param);
    // priority = param.sched_priority; 
    // printf("Thread %d priority %d\n", *((int *) arg), priority);

    /* 1. Wait until all threads are ready */
    pthread_barrier_wait(&barrier);

    clock_t volatile wake_clock = clock() + period_busy * CLOCKS_PER_SEC;
    /* 2. Do the task */ 
    for (int i = 0; i < 3; i++) {
        printf("Thread %d is running\n", *((int *) arg));
        /* Busy for <time_wait> seconds */
        while (clock() < wake_clock);
        wake_clock += (clock_t) (period_busy * CLOCKS_PER_SEC);
    }
    /* 3. Exit the function  */
    return 0;
}