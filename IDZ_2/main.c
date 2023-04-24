#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>

#define NUM_PHILOSOPHERS 5
#define THINKING 0
#define HUNGRY 1
#define EATING 2

typedef struct {
    int state[NUM_PHILOSOPHERS];
    sem_t mutex;
    sem_t sem_philosophers[NUM_PHILOSOPHERS];
} shared_data_t;

shared_data_t *shared_data;

void test(int philosopher_id) {
    if (shared_data->state[philosopher_id] == HUNGRY &&
        shared_data->state[(philosopher_id + NUM_PHILOSOPHERS - 1) % NUM_PHILOSOPHERS] != EATING &&
        shared_data->state[(philosopher_id + 1) % NUM_PHILOSOPHERS] != EATING) {
        shared_data->state[philosopher_id] = EATING;
        sem_post(&shared_data->sem_philosophers[philosopher_id]);
    }
}

void take_forks(int philosopher_id) {
    sem_wait(&shared_data->mutex);
    shared_data->state[philosopher_id] = HUNGRY;
    test(philosopher_id);
    sem_post(&shared_data->mutex);
    sem_wait(&shared_data->sem_philosophers[philosopher_id]);
}

void put_forks(int philosopher_id) {
    sem_wait(&shared_data->mutex);
    shared_data->state[philosopher_id] = THINKING;
    test((philosopher_id + NUM_PHILOSOPHERS - 1) % NUM_PHILOSOPHERS);
    test((philosopher_id + 1) % NUM_PHILOSOPHERS);
    sem_post(&shared_data->mutex);
}

void philosopher(int philosopher_id) {
    while (1) {
        printf("Philosopher %d is thinking\n", philosopher_id);
        sleep(rand() % 3 + 1);

        printf("Philosopher %d is hungry\n", philosopher_id);
        take_forks(philosopher_id);
        printf("Philosopher %d is eating\n", philosopher_id);
        sleep(rand() % 3 + 1);

        put_forks(philosopher_id);
    }
}

int main() {
    srand(time(NULL));

    int shared_memory_fd = shm_open("/philosophers_shm", O_CREAT | O_RDWR, 0666);
    if (shared_memory_fd == -1) {
        perror("shm_open");
        return 1;
    }
    if (ftruncate(shared_memory_fd, sizeof(shared_data_t)) == -1) {
        perror("ftruncate");
        return 1;
    }
    shared_data = mmap(NULL, sizeof(shared_data_t), PROT_READ | PROT_WRITE, MAP_SHARED, shared_memory_fd, 0);
    if (shared_data == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    sem_init(&shared_data->mutex, 1, 1);
    for (int i = 0; i < NUM_PHILOSOPHERS; ++i) {
        sem_init(&shared_data->sem_philosophers[i], 1, 0);
    }pid_t pids[NUM_PHILOSOPHERS];

    for (int i = 0; i < NUM_PHILOSOPHERS; ++i) {
        pids[i] = fork();
        if (pids[i] < 0) {
            perror("fork");
            return 1;
        } else if (pids[i] == 0) {
            philosopher(i);
            exit(0);
        }
    }

    for (int i = 0; i < NUM_PHILOSOPHERS; ++i) {
        waitpid(pids[i], NULL, 0);
    }

    for (int i = 0; i < NUM_PHILOSOPHERS; ++i) {
        sem_destroy(&shared_data->sem_philosophers[i]);
    }
    sem_destroy(&shared_data->mutex);

    munmap(shared_data, sizeof(shared_data_t));
    shm_unlink("/philosophers_shm");

    return 0;
}