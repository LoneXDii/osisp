#include <iostream>
#include <Windows.h>
#include "Sort.h"
#include "MergeArrays.h"

DWORD WINAPI threadSort(LPVOID);
int* generateArr(int);

struct ThreadData {
    int* arr;
    int size;
    int threadNum;
    HANDLE startEvent;
};

int main() {
    int threadsCount, arrSize;
    std::cout << "Enter threads count (from 1 to 10)\n";
    std::cin >> threadsCount;
    std::cout << "Enter arr size\n";
    std::cin >> arrSize;
    if (threadsCount < 1)
        threadsCount = 1;
    if (threadsCount > 10)
        threadsCount = 10;

    int* arr = generateArr(arrSize);

    for (int i = 0; i < arrSize; i++) {
        std::cout << arr[i] << " ";
    }
    std::cout << "\n\n";

    HANDLE* threads = new HANDLE[threadsCount];
    HANDLE startEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    int** dividedArrs = new int* [threadsCount];

    int tArrInterval = arrSize / threadsCount;
    for (int i = 0; i < threadsCount; i++) {
        int start = i * tArrInterval;
        int end = i != threadsCount - 1
                  ? ((i + 1) * (tArrInterval)) - 1
                  : arrSize - 1;

        int size = end - start + 1;
        dividedArrs[i] = new int[size];
        for (int j = start; j <= end; j++) {
            dividedArrs[i][j - start] = arr[j];
            std::cout << dividedArrs[i][j - start] << " ";
        }
        std::cout << "\n";

        ThreadData* tData = new ThreadData;
        tData->arr = dividedArrs[i];
        tData->size = size;
        tData->threadNum = i;
        tData->startEvent = startEvent;
        
        threads[i] = CreateThread(NULL, 0, threadSort, tData, 0, NULL);
    }
    SetEvent(startEvent);
    WaitForMultipleObjects(threadsCount, threads, TRUE, INFINITE);

    std::cout << "\n";
    for (int i = 0; i < threadsCount; i++) {
        int start = i * tArrInterval;
        int end = i != threadsCount - 1
            ? ((i + 1) * (tArrInterval)) - 1
            : arrSize - 1;
        int size = end - start + 1;

        for (int j = 0; j < size; j++) {
            std::cout << dividedArrs[i][j] << " ";
        }
        std::cout << "\n";
    }
    std::cout << "\n";

    for (int i = 0; i < arrSize; i++) {
        std::cout << arr[i] << " ";
    }
    std::cout << "\n";

    for (int i = 0; i < threadsCount; i++) {
        CloseHandle(threads[i]);
    }
    delete[] arr;
    delete[] threads;
	return 0;
}

DWORD WINAPI threadSort(LPVOID tData) {
    ThreadData* data = static_cast<ThreadData*>(tData);
    WaitForSingleObject(data->startEvent, INFINITE);
    mergeSort(data->arr, 0, data->size - 1);
    return 0;
}

int* generateArr(int size) {
    srand(time(0));
    int* arr = new int[size];
    for (long long i = 0; i < size; i++) {
        arr[i] = rand() % 10;
    }
    return arr;
}