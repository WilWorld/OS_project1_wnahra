#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <ctime>
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

    // Create or open shared memory object
    int fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
    if (fd == -1) { perror("shm_open"); return 1; }

    // Set the size of the shared memory object
    ftruncate(fd, sizeof(Table));

    // Map shared memory into process address space
    Table* t = static_cast<Table*>(mmap(nullptr, sizeof(Table),
                                       PROT_READ | PROT_WRITE,
                                       MAP_SHARED, fd, 0));
    if (t == MAP_FAILED) { perror("mmap"); return 1; }

    // Initialize semaphores
    sem_init(&t->full, 1, 0);          // No items initially
    sem_init(&t->empty, 1, TABLE_SIZE); // All slots empty initially
    sem_init(&t->mutex, 1, 1);         // Mutex initialized to 1

    srand(time(nullptr)); // Seed random number generator

    int produced_count = 0;

    // Produce 6 items or until Ctrl+C is pressed
    while (!stop_flag && produced_count < 6) {
        int item = rand() % 100; // Generate random item

        sem_wait(&t->empty); // Wait for an empty slot
        sem_wait(&t->mutex); // Lock table for exclusive access

        // Place item on the table
        t->table[t->item_count++] = item;
        std::cout << "Produced: " << item << std::endl;

        sem_post(&t->mutex); // Release mutex
        sem_post(&t->full);  // Signal that a new item is available

        produced_count++;
        sleep(1); // Simulate production delay
    }

    std::cout << "Producer exiting..." << std::endl;
    return 0;
}
