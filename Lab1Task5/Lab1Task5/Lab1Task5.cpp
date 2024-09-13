#include <iostream>
#include <Windows.h>
#include <algorithm>
#include <time.h>

#define ARR_SIZE 1e7

struct ThreadData {
	int* arr;
	HANDLE startEvent;
	HANDLE mutex;
};

int threadCount = 0;

void generateArr(int*);
DWORD WINAPI threadSortFunc(LPVOID);
void setCursor(int, int);

int main()
{
	

	int* arr = new int[ARR_SIZE];;
	generateArr(arr);
	HANDLE startEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	HANDLE mutex = CreateMutex(NULL, FALSE, NULL);

	ThreadData data;
	data.arr = arr;
	data.startEvent = startEvent;
	data.mutex = mutex;

	HANDLE thread1 = CreateThread(NULL, 0, threadSortFunc, &data, 0, NULL);
	if (!SetThreadPriority(thread1, THREAD_PRIORITY_NORMAL)) {
		std::cout << "thread1 err\n";
	}
	HANDLE thread2 = CreateThread(NULL, 0, threadSortFunc, &data, 0, NULL);
	if (!SetThreadPriority(thread2, THREAD_PRIORITY_ABOVE_NORMAL)) {
		std::cout << "thread2 err\n";
	}
	HANDLE thread3 = CreateThread(NULL, 0, threadSortFunc, &data, 0, NULL);
	if (!SetThreadPriority(thread3, THREAD_PRIORITY_BELOW_NORMAL)) {
		std::cout << "thread3 err\n";
	}
	HANDLE thread4 = CreateThread(NULL, 0, threadSortFunc, &data, 0, NULL);
	if (!SetThreadPriority(thread4, THREAD_PRIORITY_HIGHEST)) {
		std::cout << "thread4 err\n";
	}
	HANDLE thread5 = CreateThread(NULL, 0, threadSortFunc, &data, 0, NULL);
	if (!SetThreadPriority(thread5, THREAD_PRIORITY_LOWEST)) {
		std::cout << "thread5 err\n";
	}
	HANDLE thread6 = CreateThread(NULL, 0, threadSortFunc, &data, 0, NULL);
	if (!SetThreadPriority(thread6, THREAD_PRIORITY_TIME_CRITICAL)) {
		std::cout << "thread6 err\n";
	}
	HANDLE thread7 = CreateThread(NULL, 0, threadSortFunc, &data, 0, NULL);
	if (!SetThreadPriority(thread7, THREAD_PRIORITY_IDLE)) {
		std::cout << "thread7 err\n";
	}
	SetEvent(startEvent);

	int b;
	std::cin >> b;
	delete[] arr;
	return 0;
}

DWORD WINAPI threadSortFunc(LPVOID data)
{
	ThreadData* tData = (ThreadData*)data;
	int* param = tData->arr;
	HANDLE startEvent = tData->startEvent;
	HANDLE mutex = tData->mutex;

	//event for starting threads at the same time
	WaitForSingleObject(startEvent, INFINITE);

	int* arr = new int[ARR_SIZE];
	int priority = GetThreadPriority(GetCurrentThread());

	int consoleY = 0;
	int progress = 0;
	int progressIter = ARR_SIZE / 100;
	WaitForSingleObject(mutex, INFINITE);
	std::cout << "Thread started with priority " << priority << "\n";
	clock_t start = clock();
	for (int i = 0; i < ARR_SIZE; i++) {
		arr[i] = param[i];
	}
	std::sort(arr, arr + (int)ARR_SIZE);
	clock_t end = clock();
	std::cout << "\nSort ended in thread with priority " << priority << " in " << (double)(end - start) / CLOCKS_PER_SEC << " secs\n";
	ReleaseMutex(mutex);
	delete[] arr;
	return 0;
}

void generateArr(int* arr) {
	srand(time(0));
	for (int i = 0; i < ARR_SIZE; i++) {
		arr[i] = rand();
	}
}

void setCursor(int x = 0, int y = 0)
{
	HANDLE handle;
	COORD coordinates;
	handle = GetStdHandle(STD_OUTPUT_HANDLE);
	coordinates.X = x;
	coordinates.Y = y;
	SetConsoleCursorPosition(handle, coordinates);
}