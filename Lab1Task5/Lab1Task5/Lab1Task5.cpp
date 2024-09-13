#include <iostream>
#include <Windows.h>
#include <set>
#include <time.h>
#include <algorithm>

#define DISPLAY_THREADS_PROGRESS true
#define ARR_SIZE 1e6

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

	HANDLE threads[7];

	threads[0] = CreateThread(NULL, 0, threadSortFunc, &data, 0, NULL);
	if (!SetThreadPriority(threads[0], THREAD_PRIORITY_NORMAL)) {
		std::cout << "thread1 err\n";
	}

	threads[1] = CreateThread(NULL, 0, threadSortFunc, &data, 0, NULL);
	if (!SetThreadPriority(threads[1], THREAD_PRIORITY_ABOVE_NORMAL)) {
		std::cout << "thread2 err\n";
	}

	threads[2] = CreateThread(NULL, 0, threadSortFunc, &data, 0, NULL);
	if (!SetThreadPriority(threads[2], THREAD_PRIORITY_BELOW_NORMAL)) {
		std::cout << "thread3 err\n";
	}

	threads[3] = CreateThread(NULL, 0, threadSortFunc, &data, 0, NULL);
	if (!SetThreadPriority(threads[3], THREAD_PRIORITY_HIGHEST)) {
		std::cout << "thread4 err\n";
	}
	threads[4] = CreateThread(NULL, 0, threadSortFunc, &data, 0, NULL);
	if (!SetThreadPriority(threads[4], THREAD_PRIORITY_LOWEST)) {
		std::cout << "thread5 err\n";
	}

	threads[5] = CreateThread(NULL, 0, threadSortFunc, &data, 0, NULL);
	if (!SetThreadPriority(threads[5], THREAD_PRIORITY_TIME_CRITICAL)) {
		std::cout << "thread6 err\n";
	}

	threads[6] = CreateThread(NULL, 0, threadSortFunc, &data, 0, NULL);
	if (!SetThreadPriority(threads[6], THREAD_PRIORITY_IDLE)) {
		std::cout << "thread7 err\n";
	}

	SetEvent(startEvent);
	WaitForMultipleObjects(7, threads, TRUE, INFINITE);

	setCursor(0, 22);
	delete[] arr;
	return 0;
}

DWORD WINAPI threadSortFunc(LPVOID data)
{
	ThreadData* tData = (ThreadData*)data;
	int* param = tData->arr;
	HANDLE startEvent = tData->startEvent;
	HANDLE mutex = tData->mutex;
	std::multiset<int> set;

	//event for starting threads at the same time
	WaitForSingleObject(startEvent, INFINITE);

	int priority = GetThreadPriority(GetCurrentThread());

	if (DISPLAY_THREADS_PROGRESS) {
		WaitForSingleObject(mutex, INFINITE);
		int consoleY = threadCount;
		threadCount++;
		ReleaseMutex(mutex);

		int progress = 0;
		int perc = 0;
		long long progressIter = ARR_SIZE / 100;

		WaitForSingleObject(mutex, INFINITE);
		setCursor(0, consoleY);
		std::cout << "Thread with priority " << priority << ": [>                   ] 0%";
		ReleaseMutex(mutex);

		clock_t timeInMutex = 0;

		clock_t start = clock();
		for (long long i = 0; i < ARR_SIZE; i++) {
			set.insert(param[i]);
			if (i % progressIter == 0) {
				clock_t mutexStart = clock();
				WaitForSingleObject(mutex, INFINITE);
				clock_t mutexEnd = clock();
				timeInMutex += mutexEnd - mutexStart;

				setCursor(0, consoleY);
				perc++;
				if (perc % 5 == 0) {
					progress++;
				}
				std::cout << "Thread with priority " << priority << ": [";
				for (int j = 0; j < progress; j++) {
					std::cout << "=";
				}
				std::cout << ">";
				for (int j = progress + 1; j < 20; j++) {
					std::cout << " ";
				}
				std::cout << "] " << perc << "%";
				ReleaseMutex(mutex);
			}
		}
		clock_t end = clock();
		clock_t totalTime = (end - start) - timeInMutex;

		WaitForSingleObject(mutex, INFINITE);
		setCursor(0, consoleY + 8);
		std::cout << "Sort ended in thread with priority " << priority << " in " << (double)totalTime / CLOCKS_PER_SEC << " secs\n";
		ReleaseMutex(mutex);
	}
	else {
		std::cout << "Started thread with priority " << priority << "\n";
		int* arr = new int[ARR_SIZE];
		for (long long i = 0; i < ARR_SIZE; i++) {
			arr[i] = param[i];
		}
		clock_t start = clock();
		std::sort(arr, arr + (long long)ARR_SIZE);
		clock_t end = clock();
		std::cout << "Sort ended in thread with priority " << priority << " in " << (double)(end - start) / CLOCKS_PER_SEC << " secs\n";
	}
	return 0;
}

void generateArr(int* arr) {
	srand(time(0));
	for (long long i = 0; i < ARR_SIZE; i++) {
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