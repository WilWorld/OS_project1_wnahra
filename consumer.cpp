#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <csignal>

// Flag to handle Ctrl+C gracefully
volatile sig_atomic_t stop_flag = 0;
void handle_sigint(int) { stop_flag = 1; }

const int TABLE_SIZE = 2;

// Structure representing the shared table in memory
struct Table {
    int table[TABLE_SIZE];   // Array to store items
    int item_count = 0;      // Current number of items on the table
    sem_t full;              // Semaphore counting filled slots
    sem_t empty;             // Semaphore counting empty slots
    sem_t mutex;             // Binary semaphore to protect access to the table
};

int main() {
    // Set up signal handler for graceful termination
    signal(SIGINT, handle_sigint);

    const char* shm_name = "/shared_table";

    // Open existing shared memory object created by producer
    int fd = shm_open(shm_name, O_RDWR, 0666);
    if (fd == -1) { perror("shm_open"); return 1; }

    // Map shared memory into process address space
    Table* t = static_cast<Table*>(mmap(nullptr, sizeof(Table),
                                       PROT_READ | PROT_WRITE,
                                       MAP_SHARED, fd, 0));
    if (t == MAP_FAILED) { perror("mmap"); return 1; }

    int consumed_count = 0;

    // Consume 6 items or until Ctrl+C is pressed
    while (!stop_flag && consumed_count < 6) {
        sem_wait(&t->full);  // Wait until there is at least one item
        sem_wait(&t->mutex); // Lock table for exclusive access

        // Remove item from table
        int item = t->table[--t->item_count];
        t->table[t->item_count] = 0; // Clear consumed slot

        std::cout << "Consumed: " << item << std::endl;

        sem_post(&t->mutex); // Release mutex
        sem_post(&t->empty); // Signal that an empty slot is available

        consumed_count++;
        sleep(2); // Simulate consumption delay
    }

    std::cout << "Consumer exiting..." << std::endl;

    // Remove shared memory object (should only be done once all processes are finished)
    shm_unlink(shm_name);

    return 0;
}
