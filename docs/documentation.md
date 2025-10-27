

This project is a C++ implementation of the producer-consumer problem. It involves two separate programs, a producer and a consumer, which run as independent processes at the same time. The producer's job is to create data, while the consumer's job is to retrieve that data.

To communicate, the two processes use a block of POSIX shared memory. This shared memory acts as a "table" or buffer that both can see and access. The buffer is intentionally small and can only hold two items at once, which creates the core synchronization challenge. which was difficult to impliment compared to what you think when you read about it.

This synchronization is handled by POSIX named semaphores, which act like stop and go signals for the processes. One semaphore tracks the number of empty slots, telling the producer to wait if the buffer is full. Another semaphore tracks the number of filled slots, telling the consumer wait if the buffer is empty. A third semaphore, a mutex, acts as a lock to ensure only one process can access the buffer at any given moment, preventing them from corrupting the data.
thus enforcing mutual exclusion.

The producer runs for a set number of items and then signals it's finished by setting a "done" flag in the shared memory. The consumer continues to run until it sees this flag and has consumed all the remaining items from the buffer. This results in a graceful shutdown, after which the producer cleans up all the shared memory and semaphores it created.





Example 1:

<img width="530" height="466" alt="image" src="https://github.com/user-attachments/assets/df0dc7a2-362a-40dd-b21f-cb502319367e" />


Example 2:


<img width="620" height="465" alt="image" src="https://github.com/user-attachments/assets/3fcc9fcd-6d53-445c-ab49-2de610a69c23" /> 




