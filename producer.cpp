#include "sharedBuffer.hpp"
#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <semaphore.h>
#include <cstdlib>
#include <ctime>
#include <thread>
#include <vector>
#include <atomic>
#include <sstream>

using std::cout; using std::endl;


int main(){
    srand(time(NULL));

    //create and open shared memory

    int shm_fd = shm_open(SHARED_MEM_NAME, O_CREAT | O_RDWR, 0666 );
    if(shm_fd == -1){
        perror("smh_open failed");
        exit(EXIT_FAILURE);
    }

    //setting size of memory segment
    if(ftruncate(shm_fd, sizeof(SharedBuffer)) == -1){
        perror("ftruncate failed");
        exit(EXIT_FAILURE);
    }

    //mmap maps the shared mem obj inyo the processes adress space

    SharedBuffer* shared_data = (SharedBuffer*)mmap(0, sizeof(SharedBuffer), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    if(shared_data == MAP_FAILED){
        perror("Producer: mmap failed");
        close(shm_fd);
        shm_unlink(SHARED_MEM_NAME);
        exit(EXIT_FAILURE);
    }

    //initialize buffer indecie note that it only needs to be done by one process usually the producer
    shared_data->in = 0;
    shared_data->out = 0;

    // create/initialize semaphores
    //sem_open creates or opens POSIX semaphore
    //O_CREAT creates if it does not already exist
    //0666 is read write permision for all
    //buufer_size 2 initial value for semaphore, only two empty slots

    sem_t* empty_sem = sem_open(SEM_EMPTY_NAME, O_CREAT, 0666, BUFFER_SIZE);

    //0 is initial value for filled sem - starts with 0 filled slots
    sem_t* filled_sem = sem_open(SEM_FILLED_NAME, O_CREAT, 0666, 0);

    //1 intial value for mutex starts unlocked
    sem_t* mutex_sem = sem_open(SEM_MUTEX_NAME, O_CREAT, 0666, 1);

    sem_t* consumer_done_sem = sem_open(SEM_CONSUMER_DONE, O_CREAT, 0666, 0);

    if(empty_sem == SEM_FAILED || filled_sem == SEM_FAILED || mutex_sem == SEM_FAILED || consumer_done_sem == SEM_FAILED){
        perror("Producer: sem_open failed");

        //do clean up befor exiting
        munmap(shared_data, sizeof(SharedBuffer));
        close(shm_fd);

        exit(EXIT_FAILURE);
    }

    //actual producer loop

    int item_count = 0;
    while(item_count < 6){         //ten example items
        int item = rand() % 100;    //generate random num

        //wait(s) empty sem if its 0, this blocks producer waits for 
        cout << "producer waiting for empty slot..." << endl;
        if(sem_wait(empty_sem) == -1){
            perror("producer: sem_wait(empty_sem) failed");
            item_count = 6; 
            break;
        }

        //wait(s) mutex sem if its 0 this blocks (mutual exclusion) only one proces in cirtical section at a time
        cout << "Producer: Entering critical section...." << endl;
        if(sem_wait(mutex_sem) == -1){
            perror("Producer: sem_wait(mutex_sem) failed");
            sem_post(empty_sem); //release empty slot claimed
            item_count = 6;
            break;
        }

        //Actual Critical Section
        shared_data->buffer[shared_data->in] = item;
        cout << "Producer: Produced Item " << item << " at index " << shared_data->in << endl;
        shared_data->in = (shared_data->in + 1) % BUFFER_SIZE; //update index 

        //end of critical section

        //signal(s) mitex_sem release lock 
        if(sem_post(mutex_sem) == -1){
            perror("Producer: sem_post(mutex_sem) failed");
            item_count = 6;
            break;
        }

        // signal(s) filled_sem - one more slot is filled
        if (sem_post(filled_sem) == -1) {
            perror("Producer: sem_post(filled_sem) failed");
            item_count = 6;
            break;
        }

        item_count++;
        sleep(rand() % 2 + 1); //simulate the time it took to make an item 

    }

    //clean up
    std::cout << "Producer: Finished producing. Waiting for consumer to finish..." << std::endl;
    // Wait for the consumer to signal it's done (P operation)
    if (sem_wait(consumer_done_sem) == -1) { 
        perror("Producer: sem_wait(consumer_done)");
        // Attempt cleanup even if wait fails, as consumer might be stuck
    }

    std::cout << "Producer: Consumer signaled done. Cleaning up resources..." << std::endl;
    
    sem_close(empty_sem);
    sem_close(filled_sem);
    sem_close(mutex_sem);
    sem_close(consumer_done_sem);

    sem_unlink(SEM_EMPTY_NAME);
    sem_unlink(SEM_FILLED_NAME);
    sem_unlink(SEM_MUTEX_NAME);
    sem_unlink(SEM_CONSUMER_DONE);

    munmap(shared_data, sizeof(SharedBuffer));
    close(shm_fd);
 

    cout << "Producer finished." << endl;
    return 0;

}