#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <omp.h>
using namespace std;

void selectionSortSequential(vector<int>& arr) {
    int n = arr.size();
    for (int i = 0; i < n - 1; i++) {
        int minIndex = i;
        for (int j = i + 1; j < n; j++) {
            if (arr[j] < arr[minIndex])
                minIndex = j;
        }
        swap(arr[i], arr[minIndex]);
    }
}

void selectionSortParallel(vector<int>& arr) {
    int n = arr.size();
    for (int i = 0; i < n - 1; i++) {
        int minIndex = i;

        // Параллельный поиск минимального элемента
        #pragma omp parallel
        {
            int localMinIndex = minIndex;
            #pragma omp for nowait
            for (int j = i + 1; j < n; j++) {
                if (arr[j] < arr[localMinIndex])
                    localMinIndex = j;
            }

            // Обновляем глобальный minIndex (critical — чтобы только 1 поток записал)
            #pragma omp critical
            {
                if (arr[localMinIndex] < arr[minIndex])
                    minIndex = localMinIndex;
            }
        }

        swap(arr[i], arr[minIndex]);
    }
}

// Функция для генерации случайных чисел
vector<int> generateArray(int size) {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(1, 1000000);
    vector<int> arr(size);
    for (int i = 0; i < size; i++)
        arr[i] = dis(gen);
    return arr;
}

void testSort(int size) {
    cout << "\nРазмер массива: " << size << "\n";
    vector<int> arr1 = generateArray(size);
    vector<int> arr2 = arr1;

    auto start = chrono::high_resolution_clock::now();
    selectionSortSequential(arr1);
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> seqTime = end - start;

    start = chrono::high_resolution_clock::now();
    selectionSortParallel(arr2);
    end = chrono::high_resolution_clock::now();
    chrono::duration<double> parTime = end - start;

    cout << "Последовательная сортировка: " << seqTime.count() << " с\n";
    cout << "Параллельная сортировка:     " << parTime.count() << " с\n";
    if (parTime.count() > 0)
        cout << "Ускорение: " << seqTime.count() / parTime.count() << "x\n";
}

int main() {
    omp_set_num_threads(4);
    cout << "Параллельная сортировка выбором с OpenMP\n";
    cout << "Потоки: " << omp_get_max_threads() << "\n";

    testSort(1000);
    testSort(10000);
    return 0;
}
