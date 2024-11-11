#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <chrono>

// Количество философов
const int numPhilosophers = 5;

// Вилки (мьютексы для управления доступом к вилкам)
std::mutex forks[numPhilosophers];

// Мьютекс для синхронизации вывода
std::mutex printMutex;

// Функция для философа
void philosopher(int id) {
    while (true) {
        {
            // Философ думает
            std::lock_guard<std::mutex> lock(printMutex);
            std::cout << "Philosopher " << id << " is thinking.\n";
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        // Вилки, которые этот философ будет использовать
        int leftFork = id;
        int rightFork = (id + 1) % numPhilosophers;

        // Для предотвращения взаимной блокировки,
        // философы с четными id сначала берут левую вилку, затем правую,
        // а с нечетными id - наоборот
        if (id % 2 == 0) {
            std::lock(forks[leftFork], forks[rightFork]);
        } else {
            std::lock(forks[rightFork], forks[leftFork]);
        }

        {
            // Философ ест
            std::lock_guard<std::mutex> lock(printMutex);
            std::cout << "Philosopher " << id << " is eating.\n";
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        // Освобождаем вилки (разблокируем мьютексы)


        {
            // Философ закончил есть
            std::lock_guard<std::mutex> lock(printMutex);
            std::cout << "Philosopher " << id << " finished eating.\n";
            std::cout << "-----------------------------\n";
        }
        forks[leftFork].unlock();
        forks[rightFork].unlock();
    }
}

int main() {
    // Создаем потоки для каждого философа
    std::vector<std::thread> threads;
    for (int i = 0; i < numPhilosophers; ++i) {
        threads.push_back(std::thread(philosopher, i));
    }

    // Ждем завершения всех потоков (философов)
    for (auto& t : threads) {
        t.join();
    }

    return 0;
}