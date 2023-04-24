#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_PHILOSOPHERS 5
#define THINKING 0
#define HUNGRY 1
#define EATING 2

int state[NUM_PHILOSOPHERS];
sem_t mutex;
sem_t sem_philosophers[NUM_PHILOSOPHERS];

void test(int philosopher_id) {
    if (state[philosopher_id] == HUNGRY &&
        state[(philosopher_id + NUM_PHILOSOPHERS - 1) % NUM_PHILOSOPHERS] != EATING &&
        state[(philosopher_id + 1) % NUM_PHILOSOPHERS] != EATING) {
        state[philosopher_id] = EATING;
        sem_post(&sem_philosophers[philosopher_id]);
    }
}

void take_forks(int philosopher_id) {
    sem_wait(&mutex);
    state[philosopher_id] = HUNGRY;
    test(philosopher_id);
    sem_post(&mutex);
    sem_wait(&sem_philosophers[philosopher_id]);
}

void put_forks(int philosopher_id) {
    sem_wait(&mutex);
    state[philosopher_id] = THINKING;
    test((philosopher_id + NUM_PHILOSOPHERS - 1) % NUM_PHILOSOPHERS);
    test((philosopher_id + 1) % NUM_PHILOSOPHERS);
    sem_post(&mutex);
}

void *philosopher(void *arg) {
    int philosopher_id = *((int *)arg);

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

    pthread_t philosophers[NUM_PHILOSOPHERS];
    int philosopher_ids[NUM_PHILOSOPHERS];

    sem_init(&mutex, 0, 1);
    for (int i = 0; i < NUM_PHILOSOPHERS; ++i) {
        sem_init(&sem_philosophers[i], 0, 0);
    }

    for (int i = 0; i < NUM_PHILOSOPHERS; ++i) {
        philosopher_ids[i] = i;
        pthread_create(&philosophers[i], NULL, philosopher, &philosopher_ids[i]);
    }

    for (int i = 0; i < NUM_PHILOSOPHERS; ++i) {
        pthread_join(philosophers[i], NULL);
    }

    return 0;
}