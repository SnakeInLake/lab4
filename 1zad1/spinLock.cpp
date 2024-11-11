#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>
#include <random>
#include <string>
#include <atomic>


using namespace std;
using namespace chrono;

string sharedString;
std::atomic_flag spinlock = ATOMIC_FLAG_INIT;

char getRandomChar() {
    static random_device rd;
    static mt19937 gen(rd());
    static uniform_int_distribution<> dis(33, 126);
    return static_cast<char>(dis(gen));
}

microseconds measureTime(auto&& func) {
    const int numIterations = 100; // Уменьшено количество итераций
    auto start = high_resolution_clock::now();
    for (int i = 0; i < numIterations; ++i) {
        func();
    }
    auto end = high_resolution_clock::now();
    return duration_cast<microseconds>(end - start) / numIterations;
}

void workWithSpinLock() {
    char randomChar = getRandomChar();
    while (spinlock.test_and_set(memory_order_acquire)); // Захват spinlock
    sharedString += randomChar;
    this_thread::sleep_for(milliseconds(1)); // Добавлено ожидание
    spinlock.clear(memory_order_release); // Освобождение spinlock
}

int main() {
    const int numThreads = 4;
    const int numIterations = 5;

    vector<thread> threads;
    sharedString.clear();


    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([i, numIterations]() {
            for (int j = 0; j < numIterations; ++j) {
                auto semaphoreDuration = measureTime(workWithSpinLock);
                cout << "Thread " << i << ", Iteration " << j << ": spinLock time: " << semaphoreDuration.count() << " µs" << endl;
            }
        });
    }


    for (auto& th : threads) {
        th.join();
    }

    cout << "Final shared string: " << sharedString << endl;

    return 0;
}