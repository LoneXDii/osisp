#include <windows.h>
#include <iostream>
#include <chrono>
#include <cmath>

#define MIN_BUFFER_SIZE 2048

int lettersAsync = 0;
int letters = 0;

void CALLBACK ReadCompletionRoutine(DWORD dwErrorCode, DWORD dwNumberOfBytesTransferred, LPOVERLAPPED lpOverlapped) {
    if (dwErrorCode == 0) { 
        lpOverlapped->Offset += dwNumberOfBytesTransferred;
        char* buffer = (char*)lpOverlapped->hEvent; 
        buffer[dwNumberOfBytesTransferred] = '\0';
        for (int i = 0; i < dwNumberOfBytesTransferred; i++) {
            if ((buffer[i] >= 65 && buffer[i] <= 90) || (buffer[i] >= 97 && buffer[i] <= 122)) {
                lettersAsync++;
            }
        }
    }
    else {
        if (dwErrorCode == 38) {
            return;
        }
        std::cerr << "Error reading file: " << dwErrorCode << std::endl;
    }
}

void processAsync(int bufferSize) {
    HANDLE hFile = CreateFile(
        L"data.txt", 
        GENERIC_READ,            
        0,                        
        NULL,                     
        OPEN_EXISTING,           
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
        NULL                      
    );

    if (hFile == INVALID_HANDLE_VALUE) {
        std::cerr << "Error opening file: " << GetLastError() << std::endl;
        return;
    }

    char* buffer = new char[bufferSize];
    OVERLAPPED overlapped = { 0 };
    overlapped.hEvent = (HANDLE)buffer; 

    while (true) {
        DWORD prev_offset = overlapped.Offset;
        if (!ReadFileEx(hFile, buffer, bufferSize - 1, &overlapped, ReadCompletionRoutine)) {
            std::cerr << "Error initiating read: " << GetLastError() << std::endl;
            CloseHandle(hFile);
            return;
        }

        SleepEx(INFINITE, TRUE);
        if (prev_offset == overlapped.Offset) {
            break;
        }
    }

    CloseHandle(hFile);
    delete[] buffer;
}

void process() {
    HANDLE hFile = CreateFile(
        L"data.txt",                
        GENERIC_READ,           
        0,                      
        NULL,                   
        OPEN_EXISTING,         
        FILE_ATTRIBUTE_NORMAL,  
        NULL);                  

    if (hFile == INVALID_HANDLE_VALUE) {
        std::cerr << "Could not open file (Error: " << GetLastError() << ")." << std::endl;
        return;
    }

    DWORD fileSize = GetFileSize(hFile, NULL);
    if (fileSize == INVALID_FILE_SIZE) {
        std::cerr << "Could not get file size (Error: " << GetLastError() << ")." << std::endl;
        CloseHandle(hFile);
        return;
    }

    char* buffer = new char[fileSize + 1];
    if (!buffer) {
        std::cerr << "Memory allocation failed." << std::endl;
        CloseHandle(hFile);
        return;
    }

    DWORD bytesRead;
    if (!ReadFile(hFile, buffer, fileSize, &bytesRead, NULL)) {
        std::cerr << "Could not read file (Error: " << GetLastError() << ")." << std::endl;
        delete[] buffer;
        CloseHandle(hFile);
        return;
    }

    buffer[bytesRead] = '\0';

    for (int i = 0; i < bytesRead; i++) {
        if ((buffer[i] >= 65 && buffer[i] <= 90) || (buffer[i] >= 97 && buffer[i] <= 122)) {
            letters++;
        }
    }

    delete[] buffer;
    CloseHandle(hFile);
}

int main() {
    int i = 0;
    bool exit = false;
    while(!exit) {
        int buff_exp = pow(2, i);
        i++;
        auto startAsync = std::chrono::high_resolution_clock::now();
        if (MIN_BUFFER_SIZE * buff_exp >= 1e9) {
            exit = true;
        }
        processAsync(MIN_BUFFER_SIZE * buff_exp);
        auto endAsync = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> durationAsync = endAsync - startAsync;
        std::cout << "Async: Time: " << durationAsync.count() << " " << "Buffer size: " << MIN_BUFFER_SIZE * buff_exp << " " << "Value: " << lettersAsync << "\n";
        lettersAsync = 0;
    }

    auto start = std::chrono::high_resolution_clock::now();
    process();
    auto end = std::chrono::high_resolution_clock::now();
    
    std::chrono::duration<double> duration = end - start;
    std::cout << "Default: Time: " << duration.count() << " " << "Value: " << letters << "\n";
    return 0;
}
