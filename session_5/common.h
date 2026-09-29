#ifndef COMMON_H
#define COMMON_H

#include <semaphore.h>
#include <sys/types.h>
#include <time.h>

#define SHM_NAME "/process_monitor_shm"
#define SEM_NAME "/sem_name"

#define MAX_WORKERS 20
#define HUNG_TIMEOUT 30

enum worker_state
{
    WORKER_UNKNOWN = 0,
    WORKER_RUNNING,
    WORKER_HUNG,
    WORKER_DEAD
};

struct worker_info
{
    pid_t pid;
    int id;

    time_t last_heartbeat;

    enum worker_state state;
};

struct shared_data
{
    struct worker_info workers[MAX_WORKERS];
};

#endif
