#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>
#include <random>
#include <string>

using namespace std;
using namespace chrono;

string sharedString;
counting_semaphore<2> sharedStringSemaphore(2);

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

void workWithSemaphore() {
    char randomChar = getRandomChar();
    {
        sharedStringSemaphore.acquire();
        sharedString += randomChar;
        this_thread::sleep_for(milliseconds(10)); // Добавлено ожидание
        sharedStringSemaphore.release();
    }
}

int main() {
    const int numThreads = 4;
    const int numIterations = 5;

    vector<thread> threads;
    sharedString.clear();


    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([i, numIterations]() {
            for (int j = 0; j < numIterations; ++j) {
                auto semaphoreDuration = measureTime(workWithSemaphore);
                cout << "Thread " << i << ", Iteration " << j << ": semaphore time: " << semaphoreDuration.count() << " µs" << endl;
            }
        });
    }


    for (auto& th : threads) {
        th.join();
    }

    cout << "Final shared string: " << sharedString << endl;

    return 0;
}