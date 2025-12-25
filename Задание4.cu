#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <cuda.h>

#define THREADS 256

__device__ void merge_arrays(int* arr, int left, int mid, int right, int* temp) {
    int i = left, j = mid + 1, k = left;
    while (i <= mid && j <= right) {
        temp[k++] = (arr[i] < arr[j]) ? arr[i++] : arr[j++];
    }
    while (i <= mid) temp[k++] = arr[i++];
    while (j <= right) temp[k++] = arr[j++];
    for (i = left; i <= right; i++) arr[i] = temp[i];
}

__global__ void gpuBlockSort(int* arr, int size, int blockSize) {
    int blockId = blockIdx.x;
    int start = blockId * blockSize;
    int end = min(start + blockSize - 1, size - 1);

    // Bubble sort внутри блока (учебная сортировка)
    for (int i = start; i < end; i++) {
        for (int j = start; j < end - (i - start); j++) {
            if (arr[j] > arr[j + 1]) {
                int tmp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = tmp;
            }
        }
    }
}

void gpuMergeSort(int* arr, int size) {
    int* d_arr;
    cudaMalloc(&d_arr, size * sizeof(int));
    cudaMemcpy(d_arr, arr, size * sizeof(int), cudaMemcpyHostToDevice);

    int blockSize = 1024;
    int blocks = (size + blockSize - 1) / blockSize;

    gpuBlockSort<<<blocks, THREADS>>>(d_arr, size, blockSize);
    cudaDeviceSynchronize();

    // GPU возвращает отсортированные блоки → CPU сливает
    int* temp = new int[size];
    for (int width = blockSize; width < size; width *= 2) {
        for (int i = 0; i < size; i += 2 * width) {
            int left = i;
            int mid = min(i + width - 1, size - 1);
            int right = min(i + 2 * width - 1, size - 1);
            merge_arrays<<<1,1>>>(d_arr, left, mid, right, temp);
        }
        cudaDeviceSynchronize();
    }

    cudaMemcpy(arr, d_arr, size * sizeof(int), cudaMemcpyDeviceToHost);
    cudaFree(d_arr);
    delete[] temp;
}

std::vector<int> generateArray(int size) {
    std::vector<int> arr(size);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 1000000);
    for (int i = 0; i < size; i++)
        arr[i] = dis(gen);
    return arr;
}

int main() {
    for (int size : {10000, 100000}) {
        std::vector<int> arr = generateArray(size);
        std::vector<int> arr_cpu = arr;

        auto t1 = std::chrono::high_resolution_clock::now();
        std::sort(arr_cpu.begin(), arr_cpu.end());
        auto t2 = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> cpu_time = t2 - t1;

        t1 = std::chrono::high_resolution_clock::now();
        gpuMergeSort(arr.data(), size);
        t2 = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> gpu_time = t2 - t1;

        std::cout << "\nРазмер массива: " << size << "\n";
        std::cout << "CPU сортировка: " << cpu_time.count() << " с\n";
        std::cout << "GPU сортировка (CUDA): " << gpu_time.count() << " с\n";
        std::cout << "Ускорение: " << cpu_time.count() / gpu_time.count() << "x\n";
    }
    return 0;
}
