#include "shared.hpp"

int main() {
    // Open Shared Memory with Retry so if producer gets started after consumer will wait
    int shm_fd;
    SharedData* shared_data;
    int max_retries = 50; // 5 seconds total
    int retry_delay_us = 100000; // 100ms
    int attempt = 0;
    while (true) {
        shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
        if (shm_fd != -1) break;
        if (++attempt >= max_retries) {
            perror("shm_open (consumer)");
            std::cerr << "Consumer: Did you start the producer first?" << std::endl; //for debugging
            exit(1);
        }
        usleep(retry_delay_us);
    }

    // Map the shared memory segment
    shared_data = (SharedData*)mmap(0, sizeof(SharedData), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_data == MAP_FAILED) {
        perror("mmap (consumer)");
        exit(1);
    }

    // Open Existing Semaphores with Retry 
    sem_t *empty_sem = SEM_FAILED, *filled_sem = SEM_FAILED, *mutex_sem = SEM_FAILED;
    attempt = 0;
    while (true) {
        empty_sem = sem_open(SEM_EMPTY_NAME, 0);
        filled_sem = sem_open(SEM_FILLED_NAME, 0);
        mutex_sem = sem_open(SEM_MUTEX_NAME, 0);
        if (empty_sem != SEM_FAILED && filled_sem != SEM_FAILED && mutex_sem != SEM_FAILED) break;
        if (++attempt >= max_retries) {
            perror("sem_open (consumer)");
            std::cerr << "Consumer: Did you start the producer first?" << std::endl;
            exit(1);
        }
        usleep(retry_delay_us);
    }

    std::cout << "Consumer: Starting up..." << std::endl;

    // Consumer Loop 
    while (true) {
        // Wait for a filled slot P operation on filled_sem 
        std::cout << "Consumer: Waiting on filled slot..." << std::endl;
        sem_wait(filled_sem);

        // Wait for mutual exclusion lock active
        std::cout << "Consumer: Waiting on mutex..." << std::endl;
        sem_wait(mutex_sem);

        //enter Critical Section 
        
        // Check for shutdown condition
        // If done is true AND the buffer is empty in == out
        if (shared_data->done && shared_data->in == shared_data->out) {
            std::cout << "Consumer: Producer is done and buffer is empty. Shutting down." << std::endl;
            sem_post(mutex_sem); // Release mutex before exiting so important 
            break; // Exit the loop
        }

        int item = shared_data->buffer[shared_data->out];
        std::cout << "Consumer: Consumed item " << item << " from index " << shared_data->out << std::endl;
        shared_data->out = (shared_data->out + 1) % BUFFER_SIZE;
        // End Critical Section 

        // Signal mutual exclusion unlock the lock
        sem_post(mutex_sem);

        // Signal that a slot is now empty V operation on empty_sem
        sem_post(empty_sem);
        
        // Add a small delay to allow the producer to run
        usleep(1000);
        
         
    }

    for (int i = 0; i < BUFFER_SIZE; ++i) {
        sem_post(filled_sem);
        sem_post(empty_sem);
    }
    // Cleanup no need to unlink only in producer
    sem_close(empty_sem);
    sem_close(filled_sem);
    sem_close(mutex_sem);

    munmap(shared_data, sizeof(SharedData));
    close(shm_fd);

    std::cout << "Consumer: Shutting down." << std::endl;
    return 0;
}
