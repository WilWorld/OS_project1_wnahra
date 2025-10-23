# OS_project1_wnahra
Wil Nahra
OS project 1
10/24/2025

Topic: Producer-Consumer Problem
The producer generates items and puts items onto the table. The consumer will pick up items.
The table can only hold two items at the same time. When the table is completed, the producer
will wait. When there are no items, the consumer will wait. We use semaphores to synchronize
the producer and the consumer. Mutual exclusion should be considered. We use threads in
the producer program and consumer program. Shared memory is used for the “table”.


This project consist of a consumer.cpp and a producer.cpp
Producer: Generates items (random integers) and puts them onto a shared table (buffer).
Consumer: Picks up items from the shared table.

Buffer Constraints:
The table can only hold 2 items at a time.
If the table is full, the producer waits.
If the table is empty, the consumer waits.

Shared Memory:
shm_open and mmap allow both producer and consumer to access the same memory buffer.

Semaphores:
full -> tracks how many items are available to consume.
empty -> tracks how many empty slots remain in the buffer.
mutex -> ensures only one process modifies the buffer at a time (mutual exclusion).

Mutual Exclusion:
Only one process can write or read the buffer at a time, preventing race conditions.

To Compile:
g++ producer.cpp -pthread -o producer

g++ consumer.cpp -pthread -o consumer

./producer & ./consumer &
