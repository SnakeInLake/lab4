#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>
#include <random>
#include <string>
#include <condition_variable>

using namespace std;
using namespace chrono;

int num_threads = 5;
int finished_threads = 0;
int next_thread = 0;
std::mutex mtx;
std::condition_variable cv;

char getRandomChar() {
    static random_device rd;
    static mt19937 gen(rd());
    static uniform_int_distribution<> dis(33, 126);
    return static_cast<char>(dis(gen));
}

microseconds measureTime(auto&& func, int thread_id) {
    const int numIterations = 100;
    auto start = high_resolution_clock::now();
    for (int i = 0; i < numIterations; ++i) {
        func(thread_id, getRandomChar());
    }
    auto end = high_resolution_clock::now();
    return duration_cast<microseconds>(end - start) / numIterations;
}

void print(int thread_id, char c) {
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [&] { return next_thread == thread_id; });

    // Симуляция работы
    this_thread::sleep_for(milliseconds(10)); // Добавлено ожидание

    next_thread = (next_thread + 1) % num_threads;
    cv.notify_all();
}

void thread_finished() {
    std::unique_lock<std::mutex> lock(mtx);
    finished_threads++;
    cv.notify_all();
}

bool all_finished() {
    std::unique_lock<std::mutex> lock(mtx);
    return finished_threads == num_threads;
}

void thread_function(int id, int num_iterations) {
    for (int j = 0; j < num_iterations; ++j) {
        auto monitorDuration = measureTime(print, id);
        cout << "Thread " << id << ", Iteration " << j << ": Monitor time: " << monitorDuration.count() << " µs" << endl;
    }
    thread_finished();
}

int main() {
    int num_iterations = 5;

    std::vector<std::thread> threads;

    for (int i = 0; i < num_threads; ++i) {
        threads.push_back(std::thread(thread_function, i, num_iterations));
    }

    while (!all_finished()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    for (auto& thread : threads) {
        thread.join();
    }

    cout << "All threads finished." << std::endl;
    return 0;
}