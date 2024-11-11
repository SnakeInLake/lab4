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
std::atomic<bool> spinWaitFlag(true);

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

void workWithSpinWait(int thread_id) {
    auto start = high_resolution_clock::now();
    int spinCount = 0;
    while (spinWaitFlag.load(memory_order_relaxed)) {
        if (spinCount < 100) {
            spinCount++;
            this_thread::yield();
        } else {
            this_thread::sleep_for(nanoseconds(1));
        }
    }
   auto end = high_resolution_clock::now();
   cout << "Thread " << thread_id << ": SpinWait time: " << duration_cast<microseconds>(end - start).count() << " µs" << endl;

    spinWaitFlag.store(true, memory_order_relaxed); // Сбрасываем флаг для следующей итерации
}

int main() {
    const int numThreads = 4;
    vector<thread> threads;

    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(workWithSpinWait, i);
    }

    this_thread::sleep_for(milliseconds(50)); // Даем потокам время зайти в цикл ожидания
    spinWaitFlag.store(false, memory_order_relaxed); // Сигнализируем о завершении ожидания

    for (auto& th : threads) {
        th.join();
    }

    return 0;
}