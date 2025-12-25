#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <climits>
#include <omp.h>

using namespace std;
int main() {
    const int SIZE = 10000;
    vector<int> arr(SIZE);

    // Генерация случайных чисел
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(1, 1000000); // Случайные числа от 1 до 1,000,000

    for (int i = 0; i < SIZE; ++i) {
        arr[i] = dis(gen);
    }

    // Последовательная реализация
    auto start_seq = chrono::high_resolution_clock::now();
    int min_seq = INT_MAX;
    int max_seq = INT_MIN;
    for (int i = 0; i < SIZE; ++i) {
        if (arr[i] < min_seq) min_seq = arr[i];
        if (arr[i] > max_seq) max_seq = arr[i];
    }
    auto end_seq = chrono::high_resolution_clock::now();
    chrono::duration<double> time_seq = end_seq - start_seq;

    cout << "Последовательная реализация:\n";
    cout << "Минимальное значение: " << min_seq << "\n";
    cout << "Максимальное значение: " << max_seq << "\n";
    cout << "Время выполнения: " << time_seq.count() << " секунд\n\n";

    // Параллельная реализация с OpenMP
    auto start_omp = chrono::high_resolution_clock::now();
    int min_omp = INT_MAX;
    int max_omp = INT_MIN;

    #pragma omp parallel for reduction(min:min_omp) reduction(max:max_omp)
    for (int i = 0; i < SIZE; ++i) {
        min_omp = min(min_omp, arr[i]);
        max_omp = max(max_omp, arr[i]);
    }

    auto end_omp = chrono::high_resolution_clock::now();
    chrono::duration<double> time_omp = end_omp - start_omp;

    cout << "Параллельная реализация (OpenMP):\n";
    cout << "Минимальное значение: " << min_omp << "\n";
    cout << "Максимальное значение: " << max_omp << "\n";
    cout << "Время выполнения: " << time_omp.count() << " секунд\n\n";

    // Сравнение времени
    cout << "Сравнение времени выполнения:\n";
    cout << "Последовательная: " << time_seq.count() << " с\n";
    cout << "Параллельная: " << time_omp.count() << " с\n";
    double speedup = time_seq.count() / time_omp.count();
    cout << "Ускорение: " << speedup << "x\n\n";

    return 0;
}
