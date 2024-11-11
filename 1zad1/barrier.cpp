#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>
#include <random>
#include <string>
#include <barrier>

using namespace std;
using namespace chrono;

string sharedString;
barrier<> sharedStringBarrier(4);

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

int main() {
    sharedString.clear();
    const int numThreads = 4;
    const int numIterations = 5;
    vector<thread> threads;

    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([&]() {
            for (int j = 0; j < numIterations; ++j) {
                auto start = high_resolution_clock::now();
                char randomChar = getRandomChar();
                sharedString += randomChar;
                this_thread::sleep_for(milliseconds(10));
                sharedStringBarrier.arrive_and_wait();
                auto end = high_resolution_clock::now();
                cout << "Thread " << this_thread::get_id() << ", Iteration " << j << ": Barrier time: " << duration_cast<microseconds>(end - start).count() << " µs" << endl;
            }
        });
    }

    for (auto& th : threads) {
        th.join();
    }

    cout << "Final shared string: " << sharedString << endl;

    return 0;

}