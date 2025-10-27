# Producer-Consumer Problem

# Overview
- implimentation of producer consumer problem using:
    - POSIX shared memory, shm_open and mmap
    - POSIX name semaphores, sem_open, sem_wait, sem_post ect
    - conatins two indipendent processes, producer and consumer
    - includes a bounded buffer that holds to procsseses at a time, sharedBuffer.hpp
    - both processes will only gain access to the buffer/shared data, (critical section), when the other has exited, using semaphores for process syncornization and enforcment of mutual exclusion. When the buffer is full the producer will wait. when the buffer is empty the consumer will wait. 

## reuirements 
- I built and run in docker conatiner
- linux or unix enviorment required
- C++ 
- has the following dependancies:
    - <semaphore.h>
    - <sys/mman.h>
    - <fcntl.h>
    - <unistd.h>
    - <sys/stat.h>



## How to Build and Run

$ g++ producer.cpp -pthread -lrt -o producer
$ g++ consumer.cpp -pthread -lrt -o consumer

## Run both processes concurrently
$ ./producer & ./consumer &


## Expected 
- producer generats randome number 0-99 and places them in shared buffer with size of 2
- consumer will remove values from the buffer
- uses, empty_sem, filled_sem, mutex_sem, and consumer_done_sem
- the above counts empty lots, counts available slots, ensures mutual exclusion, and indicates whent he consumer has finished

## How it works
- The producer will:
    - create the shared memory and initializes the sephamores 
    - it will loop to produce ten items, note that only two will be put in the buffer at a time
    - producer waits on empty_sem, locks mutex_sem, then writes to the shared buffer, then will signal filled_sem
    - after it has produced all items it will wait for the consumer to finish. Once notified that the consumer has finished it will clean up necessary memory. I added the consumer_done_sem to ensure that the producer would not try to clean up memeory befor the consumer was done consuming. 

- The consumer will:
    - opens existing memory created by the producer
    - will loop to consume all items until they have all been read
    - waits for filled_sem, locks mutex_Sem, reads in item, then signals empty_sem
    - notifys producer when it has finished consuming so that producer can clean up memory

## problems
- the program should exit after both processes output finished, however i ran into some issues where the program freezes up after everything is complete, i believe this is due to on or both of the processes waiting in the background even though its complete. I tried to add in checks to ensure both processes were woken up befor exiting. However i still ran into the same issue and unfortuantley could not find a fix befor the deadline. 

# Author
Caroline Shantery: OS asignment one