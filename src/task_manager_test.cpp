#include "task_manager.hpp"
#include <chrono>
#include <thread>

using namespace pomagma;

void test_simple (size_t max_threads = 20)
{
    for (size_t i = 1; i <= max_threads; ++i) {
        POMAGMA_INFO("Starting " << i << " threads");
        TaskManager::start(i);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        TaskManager::stopall();
        POMAGMA_INFO("Stopped " << i << " threads");
    }
}

int main ()
{
    test_simple();

    return 0;
}
