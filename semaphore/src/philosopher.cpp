#include <iostream>
#include <thread>
#include <unistd.h>
#include "semaphore.h"

#define NUM_PHILOSOPHERS 5

void eatThink(int id, bool chopsticks[NUM_PHILOSOPHERS], Semaphore *semaphores[NUM_PHILOSOPHERS]);

int main(int argc, char **argv)
{
    int i;

    // initialize shared data
    Semaphore *semaphores[NUM_PHILOSOPHERS];
    bool chopsticks[NUM_PHILOSOPHERS];
    for (i = 0; i < NUM_PHILOSOPHERS; i++)
    {
        semaphores[i] = new Semaphore(1);
        chopsticks[i] = true;
    }

    // start each philosopher in its own thread
    std::thread philosophers[NUM_PHILOSOPHERS];
    for (i = 0; i < NUM_PHILOSOPHERS; i++)
    {
        philosophers[i] = std::thread(eatThink, i, chopsticks, semaphores);
    }

    // wait for all threads to complete
    for (i = 0; i < NUM_PHILOSOPHERS; i++)
    {
        philosophers[i].join();
    }

    return 0;
}

void eatThink(int id, bool chopsticks[NUM_PHILOSOPHERS], Semaphore *semaphores[NUM_PHILOSOPHERS])
{
    int left = id;
    int right = (id + 1) % NUM_PHILOSOPHERS;

    usleep(10000 * id); // 0.01 second offset per philosopher

    while (true)
    {
        // 'think' for 1.5 seconds (i.e. do nothing)
        std::cout << "Philosopher " << id << ": thinking!" << std::endl;
        usleep(1500000);

        // pick up left chopstick
        // pick up right chopstick
        // eat for 0.75 seconds
        // set down left chopstick
        // set down right chopstick
        std::cout << "Philosopher " << id << ": ready to eat..." << std::endl;
        semaphores[left]->wait();
        chopsticks[left] = false;
        // add artificial wait (0.1 seconds) here to ensure deadlock condition happens
        semaphores[right]->wait();
        chopsticks[right] = false;

        std::cout << "Philosopher " << id << ": eating!" << std::endl;
        usleep(750000);

        chopsticks[left] = true;
        semaphores[left]->signal();
        chopsticks[right] = true;
        semaphores[right]->signal();
    }
}
