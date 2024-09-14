#include <iostream>
#include <Windows.h>

DWORD WINAPI threadSort(LPVOID);
void merge(int*, int, int, int);
void mergeSort(int*, int, int);
int* generateArr(int);

struct ThreadData {
    int* arr;
    int start;
    int end;
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
    std::cout << "\n";

    HANDLE* threads = new HANDLE[threadsCount];
    HANDLE startEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    int tArrInterval = arrSize / threadsCount;
    for (int i = 0; i < threadsCount; i++) {
        ThreadData* tData = new ThreadData;
        tData->arr = arr;
        tData->start = i * tArrInterval;
        tData->end = i != threadsCount - 1
                    ? ((i + 1) * (tArrInterval)) - 1
                    : arrSize - 1;
        tData->threadNum = i + 1;
        tData->startEvent = startEvent;
        
        threads[i] = CreateThread(NULL, 0, threadSort, tData, 0, NULL);
    }

    SetEvent(startEvent);
    WaitForMultipleObjects(threadsCount, threads, TRUE, INFINITE);

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
    mergeSort(data->arr, data->start, data->end);
    return 0;
}

void merge(int* arr, int left, int mid, int right) {
    int n1 = mid - left + 1;
    int n2 = right - mid;

    int* L = new int[n1];
    int* R = new int[n2];

    for (int i = 0; i < n1; ++i)
        L[i] = arr[left + i];
    for (int j = 0; j < n2; ++j)
        R[j] = arr[mid + 1 + j];

    int i = 0, j = 0, k = left;
    while (i < n1 && j < n2) {
        if (L[i] <= R[j]) {
            arr[k] = L[i];
            ++i;
        }
        else {
            arr[k] = R[j];
            ++j;
        }
        ++k;
    }

    while (i < n1) {
        arr[k] = L[i];
        ++i;
        ++k;
    }

    while (j < n2) {
        arr[k] = R[j];
        ++j;
        ++k;
    }

    delete[] L;
    delete[] R;
}

void mergeSort(int* arr, int left, int right) {
    if (left < right) {
        int mid = left + (right - left) / 2;

        mergeSort(arr, left, mid);
        mergeSort(arr, mid + 1, right);

        merge(arr, left, mid, right);
    }
}

int* generateArr(int size) {
    srand(time(0));
    int* arr = new int[size];
    for (long long i = 0; i < size; i++) {
        arr[i] = rand() % 10;
    }
    return arr;
}