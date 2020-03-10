#include <mutex>
#include <condition_variable>

class Semaphore
{
private:
    std::mutex mutex;
    std::condition_variable condition;
    uint32_t count;

public:
    Semaphore(uint32_t initial_count);
    ~Semaphore() {};

    void signal();
    void wait(uint32_t num = 1);
    bool tryWait(uint32_t num = 1);
};
