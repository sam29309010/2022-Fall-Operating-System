Scheduling Policy Demonstration Program
===

###### tags: `OS`

* [Assignment 2: Scheduling Policy Demonstration Program](https://hackmd.io/@Cycatz/HyhStPHHj)

### Implementation of specifying thread policy and prioirty
> Describe how you implemented the program in detail.

The implementation of main thread could be divided into six steps:
1. Parse program arguments

    Use `getopt` to parse the command line options and `strtok` to split & identify each thread's scheduling policy/priority.
    ```cpp
    thread_info_t thread_infos[MAXTHREAD];
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
    ```
2. Create <num_threads> worker threads

    Use `pthread_barrier_init` to initialize a barrier to block the threads immediately after creation. Then, call `pthread_create` to create a given number of threads by default attributes.
    ```cpp
    pthread_barrier_init(&barrier, NULL, num_thread + 1);
    for (int i = 0; i < num_thread; i++)
    {
        thread_infos[i].printed_thread_id = i;
        pthread_create(&thread_infos[i].thread_id, NULL, thread_func, (void *) &thread_infos[i].printed_thread_id);
    }
    ```

    
3. Set CPU affinity

    Set the CPU affinity of the main thread to number zero and set the affinity of other threads to number zero, both by using `pthread_setaffinity_np`.
    ```cpp
    cpu_set_t cpuset_zero, cpuset_one;
    CPU_ZERO(&cpuset_zero);
    CPU_SET(0, &cpuset_zero);
    pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset_zero);
    
    CPU_ZERO(&cpuset_one);
    CPU_SET(1, &cpuset_one);
    for (int i = 0; i < num_thread; i++) {
        pthread_setaffinity_np(thread_infos[i].thread_id, sizeof(cpu_set_t), &cpuset_one); 
    }

    ```
4. Set the attributes to each thread

    Set the scheduling policy and priority for each thread using `pthread_setschedparam`. Note the difference between function `pthread_setschedparam` and `pthread_attr_setschedparam`.
    ```cpp
    sched_param params[MAXTHREAD];
    for (int i = 0; i < num_thread; i++) {
        params[i].sched_priority = (thread_infos[i].sched_priority >= 0) ? thread_infos[i].sched_priority : 0;
        pthread_setschedparam(thread_infos[i].thread_id, thread_infos[i].sched_policy, &params[i]);        
    }    
    ```
5. Start all threads at once

    The created threads would be blocked first by the barrier. Restart all of them here since the attributes of each thread have already been modified.
    ```cpp
    pthread_barrier_wait(&barrier);
    ```
6. Wait for all threads to finish

    Wait until all created thread finish executing `thread_func`
    ```cpp
    for (int i = 0; i < num_thread; i++)
        pthread_join(thread_infos[i].thread_id, NULL);
    ```

The implementation of thread function *thread_func* could be divided into two steps:
1. Wait until all threads are ready

    Explicitly block the thread itself until all threads have been created and set.
    ```cpp
    pthread_barrier_wait(&barrier);
    ```
2. Do the task

    Print out the required text and do busy-waiting three times for a given period.
    ```cpp
    clock_t volatile wake_clock = clock() + period_busy * CLOCKS_PER_SEC;
    for (int i = 0; i < 3; i++) {
        printf("Thread %d is running\n", *((int *) printed_thread_id));
        /* Busy for <time_wait> seconds */
        while (clock() < wake_clock);
        wake_clock += (clock_t) (period_busy * CLOCKS_PER_SEC);
    }
    ```

### Cases Discussion
> Describe the results of `./sched_demo -n 3 -t 1.0 -s NORMAL,FIFO,FIFO -p -1,10,30` and what causes that.

The result of command `./sched_demo -n 3 -t 1.0 -s NORMAL,FIFO,FIFO -p -1,10,30` is shown below.
```
Thread 2 is running
Thread 2 is running
Thread 2 is running
Thread 1 is running
Thread 1 is running
Thread 1 is running
Thread 0 is running
Thread 0 is running
Thread 0 is running
```
Three threads have been specified. Thread 1 and 2 are set as real-time processes scheduled by FIFO policy. Since Thread 2 has a higher priority than Thread 1, Thread 2 is executed first, followed by Thread 1. Thread 0 shall start until finishing all the real-time processes. 

> Describe the results of `./sched_demo -n 4 -t 0.5 -s NORMAL,FIFO,NORMAL,FIFO -p -1,10,-1,30` and what causes that.

The result of command `./sched_demo -n 4 -t 0.5 -s NORMAL,FIFO,NORMAL,FIFO -p -1,10,-1,30` is shown below.
```
Thread 3 is running
Thread 3 is running
Thread 3 is running
Thread 1 is running
Thread 1 is running
Thread 1 is running
Thread 2 is running
Thread 0 is running
Thread 2 is running
Thread 0 is running
Thread 2 is running
Thread 0 is running
```
Four threads have been specified. Thread 1 and 3 are set as real-time processes that are scheduled by FIFO policy. Since Thread 3 has a higher priority than Thread 1, Thread 3 is executed first, followed by Thread 1. Thread 0 and 2 shall start until finishing all the real-time processes. They are scheduled by SCHED_OTHER, i.e., CFS policy, so both processes take turns executeing for a given time and then being switched out.

### N-second-busy-waiting
> Describe how did you implement n-second-busy-waiting?

The n-second-busy-waiting is implemented by iteratively calling the function `clock()` to check the elapsed time. In this way, the process will keep comparing the current time with our specified waiting time without being switched to the sleep state by the kernel.

```cpp
while (clock() < wake_clock);
    wake_clock += (clock_t) (period_busy * CLOCKS_PER_SEC);
```