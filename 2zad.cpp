#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <thread>
#include <mutex>
#include <chrono>
#include <algorithm>
#include <numeric>

using namespace std;
using namespace chrono;

struct Employee {
    string name;
    string position;
    int age;
    double salary;
};

vector<Employee> generateData(size_t size) {
    vector<Employee> data(size);
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> ageDist(20, 60);
    uniform_real_distribution<> salaryDist(30000.0, 100000.0);
    vector<string> names = {"John Doe", "Jane Smith", "Alice Johnson", "Bob Brown"};
    vector<string> positions = {"Engineer", "Manager", "Analyst", "Developer"};

    for (auto& emp : data) {
        emp.name = names[gen() % names.size()];
        emp.position = positions[gen() % positions.size()];
        emp.age = ageDist(gen);
        emp.salary = salaryDist(gen);
    }
    return data;
}

// Последовательная обработка данных
pair<double, double> sequentialProcessing(const vector<Employee>& data, const string& targetPosition, vector<Employee>& resultEmployees) {
    vector<Employee> filtered; // Вектор для хранения отфильтрованных сотрудников
    double totalAge = 0; // Суммарный возраст отфильтрованных сотрудников
    int count = 0; // Количество отфильтрованных сотрудников

    // Фильтрация сотрудников по должности
    for (const auto& emp : data) {
        if (emp.position == targetPosition) {
            filtered.push_back(emp);
            totalAge += emp.age;
            count++;
        }
    }

    double averageAge = count ? totalAge / count : 0; // Расчет среднего возраста
    double maxSalary = 0; // Максимальная зарплата среди отфильтрованных сотрудников

    // Поиск максимальной зарплаты среди сотрудников с возрастом, близким к среднему
    for (const auto& emp : filtered) {
        if (abs(emp.age - averageAge) <= 2) {
            maxSalary = max(maxSalary, emp.salary);
            resultEmployees.push_back(emp); // Добавление сотрудника в результирующий вектор
        }
    }

    return {averageAge, maxSalary}; // Возврат среднего возраста и максимальной зарплаты
}

// Параллельная обработка данных
pair<double, double> parallelProcessing(const vector<Employee>& data, const string& targetPosition, int numThreads, vector<Employee>& resultEmployees) {
    vector<thread> threads; // Вектор для хранения потоков
    mutex mtx; // Мьютекс для синхронизации доступа к общим данным
    vector<Employee> filtered; // Вектор для хранения отфильтрованных сотрудников
    double totalAge = 0; // Суммарный возраст отфильтрованных сотрудников
    int count = 0; // Количество отфильтрованных сотрудников

    // Лямбда-функция для выполнения работы в каждом потоке
    auto worker = [&](size_t start, size_t end) {
        vector<Employee> localFiltered; // Локальный вектор для хранения отфильтрованных сотрудников
        double localTotalAge = 0; // Локальный суммарный возраст
        int localCount = 0; // Локальное количество отфильтрованных сотрудников

        // Фильтрация данных в пределах заданного диапазона
        for (size_t i = start; i < end; ++i) {
            if (data[i].position == targetPosition) {
                localFiltered.push_back(data[i]);
                localTotalAge += data[i].age;
                localCount++;
            }
        }

        // Блок синхронизации для добавления локальных результатов к общим
        lock_guard<mutex> lock(mtx); 
        filtered.insert(filtered.end(), localFiltered.begin(), localFiltered.end());
        totalAge += localTotalAge;
        count += localCount;
    };

    // Разделение данных на части и запуск потоков
    size_t chunkSize = data.size() / numThreads;
    for (int i = 0; i < numThreads; ++i) {
        size_t start = i * chunkSize;
        size_t end = (i == numThreads - 1) ? data.size() : start + chunkSize; // Обработка последней части данных
        threads.emplace_back(worker, start, end); // Запуск потока
    }

    // Ожидание завершения всех потоков
    for (auto& th : threads) {
        th.join();
    }

    double averageAge = count ? totalAge / count : 0; // Расчет среднего возраста
    double maxSalary = 0; // Максимальная зарплата среди отфильтрованных сотрудников

    // Поиск максимальной зарплаты среди сотрудников с возрастом, близким к среднему
    for (const auto& emp : filtered) {
        if (abs(emp.age - averageAge) <= 2) {
            maxSalary = max(maxSalary, emp.salary);
            resultEmployees.push_back(emp); // Добавление сотрудника в результирующий вектор
        }
    }

    return {averageAge, maxSalary};
}

void printEmployees(const vector<Employee>& employees) {
    for (const auto& emp : employees) {
        cout << "Name: " << emp.name << ", Position: " << emp.position 
             << ", Age: " << emp.age << ", Salary: " << emp.salary << endl;
    }
}

int main() {
    size_t dataSize = 10000000;
    int numThreads = 4;
    string targetPosition = "Developer";

    // Generate data
    vector<Employee> data = generateData(dataSize);

    // Sequential processing
    vector<Employee> sequentialResultEmployees;
    auto start = high_resolution_clock::now();
    auto sequentialResult = sequentialProcessing(data, targetPosition, sequentialResultEmployees);
    auto end = high_resolution_clock::now();
    auto sequentialDuration = duration_cast<milliseconds>(end - start).count();

    // Parallel processing
    vector<Employee> parallelResultEmployees;
    start = high_resolution_clock::now();
    auto parallelResult = parallelProcessing(data, targetPosition, numThreads, parallelResultEmployees);
    end = high_resolution_clock::now();
    auto parallelDuration = duration_cast<milliseconds>(end - start).count();

    // Output results
    cout << "Sequential Processing: " << sequentialDuration << " ms" << endl;
    cout << "Parallel Processing: " << parallelDuration << " ms" << endl;

    cout << "Sequential Result: Average Age = " << sequentialResult.first
         << ", Max Salary = " << sequentialResult.second << endl;
    cout << "Parallel Result: Average Age = " << parallelResult.first
         << ", Max Salary = " << parallelResult.second << endl;

    // Verify that results are the same
    if (sequentialResult == parallelResult) {
        cout << "Results are consistent." << endl;
    } else {
        cout << "Results are inconsistent!" << endl;
    }

    /*// Print filtered employees
    cout << "\nFiltered Employees (Sequential):" << endl;
    printEmployees(sequentialResultEmployees);

    cout << "\nFiltered Employees (Parallel):" << endl;
    printEmployees(parallelResultEmployees);*/

    return 0;
}