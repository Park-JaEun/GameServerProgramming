#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <mutex>
#include <atomic>

using namespace std::chrono;

volatile int g_sum = 0;

std::mutex sum_lock;
//std::atomic<std::
struct POS {
	int x;
	int y;
};


void thread_worker(const int num_th)
{
    volatile int local_sum = 0;
    for (int i = 0; i < 50000000 / num_th; ++i) {
        local_sum = local_sum + 2;
    }
    sum_lock.lock();
    g_sum = g_sum + local_sum;
    sum_lock.unlock();
}

std::atomic<int> a_sum;
void a_thread_worker(const int num_th)
{
    for (int i = 0; i < 50000000 / num_th; ++i)
        a_sum += 2;
}

int main()
{
    {
        volatile int sum = 0;

        auto start_t = high_resolution_clock::now();
        for (int i = 0; i < 50000000; ++i) {
            sum = sum + 2;
        }
        auto end_t = high_resolution_clock::now();
        auto exec_t = end_t - start_t;
        auto exec_ms = duration_cast<milliseconds>(exec_t).count();

        std::cout << "SUM = " << sum << ", time = " << exec_ms << "ms.\n";
    }
    {
        for (int num_threads = 1; num_threads <= 16; num_threads *= 2) {
            g_sum = 0;
            std::vector<std::thread> threads;
            auto start_t = high_resolution_clock::now();
            for (int i = 0; i < num_threads; ++i)
                threads.emplace_back(thread_worker, num_threads);
            for (auto& th : threads)
                th.join();
            auto end_t = high_resolution_clock::now();
            auto exec_t = end_t - start_t;
            auto exec_ms = duration_cast<milliseconds>(exec_t).count();

            std::cout << num_threads << " Thread SUM = " << g_sum << ", time = " << exec_ms << "ms.\n";
        }
    }
    {
        for (int num_threads = 1; num_threads <= 16; num_threads *= 2) {
            a_sum = 0;
            std::vector<std::thread> threads;
            auto start_t = high_resolution_clock::now();
            for (int i = 0; i < num_threads; ++i)
                threads.emplace_back(a_thread_worker, num_threads);
            for (auto& th : threads)
                th.join();
            auto end_t = high_resolution_clock::now();
            auto exec_t = end_t - start_t;
            auto exec_ms = duration_cast<milliseconds>(exec_t).count();

            std::cout << num_threads << " Thread Atomic SUM = " << a_sum << ", time = " << exec_ms << "ms.\n";
        }
    }
    //std::vector<std::thread> my_threads;
    //for (int i = 0; i < 16; ++i) {
    //   my_threads.emplace_back(thread_worker, i);
    //}
    //for (auto& th : my_threads)
    //   th.join();
}