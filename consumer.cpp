#include "sharedBuffer.hpp" 
#include <iostream>
#include <unistd.h>     // For sleep, close
#include <fcntl.h>      // For O_* constants
#include <sys/mman.h>   // For shared memory (mmap, shm_open)
#include <sys/stat.h>   // For mode constants
#include <semaphore.h>  // For POSIX semaphores
#include <cstdlib>      // For rand, srand, exit
#include <ctime>        // For time
using std::cout; using std::endl;

int main(){
    srand(time(NULL) + getpid()); //seed this diferently using proccess id

    //open and map the existing shared memory
    //consuming so no need to create we want existing segment
    int shm_fd = shm_open(SHARED_MEM_NAME, O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("Consumer: shm_open failed");
        exit(EXIT_FAILURE);
        cout << "Start the producer first so shared memory and semaphores exist." << endl;
    }

    // map the shared memory object
    SharedBuffer* shared_data = (SharedBuffer*)mmap(0, sizeof(SharedBuffer), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    if (shared_data == MAP_FAILED) {
        perror("Consumer: mmap failed");
        close(shm_fd);
        exit(EXIT_FAILURE);
    }

    //open existing sephamores
    sem_t* empty_sem = sem_open(SEM_EMPTY_NAME, 0);
    sem_t* filled_sem = sem_open(SEM_FILLED_NAME, 0);
    sem_t* mutex_sem = sem_open(SEM_MUTEX_NAME, 0);
    sem_t* consumer_done_sem = sem_open(SEM_CONSUMER_DONE, 0);

    if (empty_sem == SEM_FAILED || filled_sem == SEM_FAILED || mutex_sem == SEM_FAILED || consumer_done_sem == SEM_FAILED) {
        perror("Consumer: sem_open failed");
        munmap(shared_data, sizeof(SharedBuffer));
        close(shm_fd);
        exit(EXIT_FAILURE);
    }

    //loop for consumer
    int consumed_count = 0;
    while (consumed_count < 6) { // Consume 10 items as an example

        // wait(s) filled_sem. If it's 0, blocks (consumer waits for item).
        cout << "Consumer: Waiting for a filled slot..." << endl;
         if (sem_wait(filled_sem) == -1) {
            perror("Consumer: sem_wait(filled_sem) failed"); break;
        }

        // Wait(s) mutex_sem. If it's 0, blocks (mutual exclusion)
        cout << "Consumer: Entering critical section..." << endl;
        if (sem_wait(mutex_sem) == -1) {
            perror("Consumer: sem_wait(mutex_sem) failed");
            sem_post(filled_sem); // Release the filled slot claimed
            break;
        }

        // actuall critical section 
        int item = shared_data->buffer[shared_data->out];
        std::cout << "Consumer: Consumed item " << item << " from index " << shared_data->out << std::endl;
        shared_data->out = (shared_data->out + 1) % BUFFER_SIZE; // update index
        // end of critical sectoin

        // signal(s) mutex_sem release the lock
        if (sem_post(mutex_sem) == -1) {
            perror("Consumer: sem_post(mutex_sem) failed"); break;
        }
        cout << "Consumer: Exited critical section." << endl;

        // signal(s) empty_sem one more slot is empty
        if (sem_post(empty_sem) == -1) {
             perror("Consumer: sem_post(empty_sem) failed"); break;
        }


        consumed_count++;
        sleep(rand() % 2 + 1); // shpws time taken to consume an item
    }

    cout << "Consumer: Finished consuming. Signaling producer..." << endl;
    if (sem_post(consumer_done_sem) == -1) { 
        perror("Consumer: sem_post(consumer_done)");
    } else {
        cout << "Consumer: Successfully signaled producer." << endl;
    }
    cout << "Consumer: Cleaning up..." << endl;
    // Close semaphores
    sem_close(empty_sem);
    sem_close(filled_sem);
    sem_close(mutex_sem);

    // Unmap shared memory
    munmap(shared_data, sizeof(SharedBuffer));
    // Close shared memory file descriptor
    close(shm_fd);

    std::cout << "Consumer finished." << std::endl;
    return 0;
    
}

