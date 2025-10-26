#ifndef SHAREDBUFFER_HPP
#define SHAREDBUFFER_HPP

#include <semaphore.h>

#define SHARED_MEM_NAME "/pc_shared_buffer"
#define SEM_EMPTY_NAME  "/pc_sem_empty"
#define SEM_FILLED_NAME "/pc_sem_filled"
#define SEM_MUTEX_NAME  "/pc_sem_mutex"
#define SEM_CONSUMER_DONE "/pc_sem_consumer_done"
#define BUFFER_SIZE 2

// Define the structure to be placed in shared memory
struct SharedBuffer {
    int buffer[BUFFER_SIZE];
    int in;  // Index for producer to insert
    int out; // Index for consumer to remove
    bool done;
};

#endif 