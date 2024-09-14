#include <iostream>
#include <Windows.h>


void merge(int*, int, int, int);
void mergeSort(int*, int, int);
int* generateArr(int);

int main() {
    int* arr = generateArr(25);
    for (int i = 0; i < 25; i++) {
        std::cout << arr[i] << " ";
    }
    std::cout << "\n\n";
    mergeSort(arr, 0, 25 - 1);
    for (int i = 0; i < 25; i++) {
        std::cout << arr[i] << " ";
    }
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
        arr[i] = rand();
    }
    return arr;
}