#ifndef SHARED_HPP
#define SHARED_HPP

// POSIX libraries for shared memory and semaphores
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>   
#include <unistd.h>     
#include <semaphore.h>  
#include <iostream>
#include <cstdlib>
#include <ctime>
#define BUFFER_SIZE 2
#define NUM_ITEMS 10


const char* SHM_NAME = "/pa1_shm";
const char* SEM_EMPTY_NAME = "/pa1_sem_empty";   
const char* SEM_FILLED_NAME = "/pa1_sem_filled";  
const char* SEM_MUTEX_NAME = "/pa1_sem_mutex";    

// This is the structure that will be placed in shared memory
struct SharedData {
    int buffer[BUFFER_SIZE];
    int in;  // Index for producer to write
    int out; // Index for consumer to read
    bool done; // Flag for shutdown
};

#endif 