#include "semaphore.h"

Semaphore::Semaphore(uint32_t initial_count) {
    count = initial_count;
}

void Semaphore::signal() {
    std::lock_guard<std::mutex> lock(mutex);
    count++;
    condition.notify_one();
}

void Semaphore::wait(uint32_t num) {
    std::unique_lock<std::mutex> lock(mutex);
    while (count < num) // handle spurious wake-ups
    {
        condition.wait(lock);
    }
    count -= num;
    lock.unlock();
}

bool Semaphore::tryWait(uint32_t num) {
    std::lock_guard<std::mutex> lock(mutex);
    bool success = false;
    if (count >= num)
    {
        count -= num;
        success = true;
    }
    return success;
}
