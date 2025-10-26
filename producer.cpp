#include "shared.hpp"

int main() {
    //Clean up old resources in case there is a crash
    // This is good practice
    shm_unlink(SHM_NAME);
    sem_unlink(SEM_EMPTY_NAME);
    sem_unlink(SEM_FILLED_NAME);
    sem_unlink(SEM_MUTEX_NAME);

    // Create Shared Memory 
    int shm_fd;
    SharedData* shared_data;

    // Create the shared memory segment 
    shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(1);
    }

    // Set the size of the shared memory
    ftruncate(shm_fd, sizeof(SharedData));

    // Map the shared memory segment into this process's address space
    shared_data = (SharedData*)mmap(0, sizeof(SharedData), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_data == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    //  Create and Initialize Semaphores 
    sem_t *empty_sem, *filled_sem, *mutex_sem;

    // sem_open with O_CREAT creates a new semaphore
    // Format: name, flags, permissions, initial_value
    
    // empty_sem ia Initialized to BUFFER_SIZE which is 2
    empty_sem = sem_open(SEM_EMPTY_NAME, O_CREAT, 0666, BUFFER_SIZE);
    
    // filled_sem is Initialized to 0 no items are in the buffer yet
    filled_sem = sem_open(SEM_FILLED_NAME, O_CREAT, 0666, 0);
    
    // mutex_sem is Initialized to 1 binary semaphore for mutual exclusion
    mutex_sem = sem_open(SEM_MUTEX_NAME, O_CREAT, 0666, 1);

    if (empty_sem == SEM_FAILED || filled_sem == SEM_FAILED || mutex_sem == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }

    //Initialize Shared Data 
    shared_data->in = 0;
    shared_data->out = 0;
    shared_data->done = false;
    srand(time(NULL)); // Seed random number generator

    // Ensure all resources are properly initialized before continuing
    //this was more for debugging purposes im going to leave it though, asked professor ma to take a look
    usleep(100000); // 100ms delay to ensure cleanup and initialization is complete
    
    std::cout << "Producer: Starting up..." << std::endl;

    // Producer Loop 
    for (int i = 0; i < NUM_ITEMS; ++i) {
        int item = rand() % 100 + 1; // Produce a random item

        // Wait for an empty slot P operation on empty_sem
        std::cout << "Producer: Waiting on empty slot..." << std::endl;
        sem_wait(empty_sem);

        // Wait for mutual exclusion lock active
        std::cout << "Producer: Waiting on mutex..." << std::endl;
        sem_wait(mutex_sem);

        // enter the Critical Section 
        shared_data->buffer[shared_data->in] = item;
        std::cout << "Producer: Produced item " << item << " at index " << shared_data->in << std::endl;
        shared_data->in = (shared_data->in + 1) % BUFFER_SIZE;
        // End Critical Section

        // Signal mutual exclusion unlock the lock
        sem_post(mutex_sem);

        // Signal that a slot is now filled V operation on filled_sem
        sem_post(filled_sem);
        
        
    }

    // Signal Shutdown
    std::cout << "Producer: Finished producing. Setting done flag." << std::endl;
    sem_wait(mutex_sem);
    shared_data->done = true;
    sem_post(mutex_sem);

    // Wake up any processes that might be blocked on 'filled' or 'empty'
    // Post enough times to unblock all potential waiters buffer size
    for (int i = 0; i < BUFFER_SIZE; ++i) {
        sem_post(filled_sem);
    }
    for (int i = 0; i < BUFFER_SIZE; ++i) {
        sem_post(empty_sem);
    }

    // Cleanup only here
    sem_close(empty_sem);
    sem_close(filled_sem);
    sem_close(mutex_sem);

    // Unlink semaphores
    sem_unlink(SEM_EMPTY_NAME);
    sem_unlink(SEM_FILLED_NAME);
    sem_unlink(SEM_MUTEX_NAME);

    // Unmap and unlink shared memory
    munmap(shared_data, sizeof(SharedData));
    close(shm_fd);
    shm_unlink(SHM_NAME);

    std::cout << "Producer: Cleaned up and shutting down." << std::endl;
    return 0;
}