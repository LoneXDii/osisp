#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <iostream>
#include <thread>
#include <string>
#include <atomic>
#include <ctime>

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"
std::atomic<bool> isRunning(true);

SOCKET ConnectSocket = INVALID_SOCKET;

std::string static GenRandom(const int len) {
    srand(static_cast<unsigned int>(time(0)));
    static const char alphanum[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::string tmp_s;
    tmp_s.reserve(len);

    for (int i = 0; i < len; ++i) {
        tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];
    }

    return tmp_s;
}

void static ReceiveMessages() {
    int iResult;
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;
    while (isRunning) {
        iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
        if (iResult > 0) {
            recvbuf[iResult] = '\0';
            std::cout << "Server message: " << recvbuf << std::endl;
        }
        else if (iResult == 0) {
            std::cout << "Connection closed by server." << std::endl;
            break;
        }
        else {
            std::cout << "Receive failed: " << WSAGetLastError() << std::endl;
            break;
        }
    }
}

void static SendMessages() {
    int iResult;
    char sendbuf[DEFAULT_BUFLEN];
    while (true) {
        std::cout << " -> Enter msg: ";
        std::cin.getline(sendbuf, sizeof(sendbuf));

        size_t len = strlen(sendbuf);
        if (len > 0 && sendbuf[len - 1] == '\n') {
            sendbuf[len - 1] = '\0';
        }

        if (strlen(sendbuf) == 0) {
            sendbuf[0] = '\0';
            iResult = send(ConnectSocket, sendbuf, 1, 0);
            if (iResult == SOCKET_ERROR) {
                std::cout << "Send failed: " << WSAGetLastError() << std::endl;
                isRunning = false;
                break;
            }
            isRunning = false;
            break;
        }
        else {
            iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
            if (iResult == SOCKET_ERROR) {
                std::cout << "Send failed: " << WSAGetLastError() << std::endl;
                break;
            }
        }
    }
}

int main() {
    WSADATA wsaData;
    int iResult;

    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        std::cout << "WSAStartup failed with error: " << iResult << std::endl;
        return 1;
    }

    struct addrinfo* result = NULL, * ptr = NULL, hints;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    iResult = getaddrinfo("127.0.0.1", DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        std::cout << "getaddrinfo failed: " << iResult << std::endl;
        WSACleanup();
        return 1;
    }

    ConnectSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ConnectSocket == INVALID_SOCKET) {
        std::cout << "Error at socket(): " << WSAGetLastError() << std::endl;
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    iResult = connect(ConnectSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        closesocket(ConnectSocket);
        ConnectSocket = INVALID_SOCKET;
    }

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) {
        std::cout << "Unable to connect to server!" << std::endl;
        WSACleanup();
        return 1;
    }

    std::cout << "Connected to server." << std::endl;

    std::string sendbufID = GenRandom(20);
    std::cout << "Generated id: " << sendbufID << std::endl;

    iResult = send(ConnectSocket, sendbufID.c_str(), 20, 0);
    if (iResult == SOCKET_ERROR) {
        std::cout << "Send failed: " << WSAGetLastError() << std::endl;
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Bytes sent: " << iResult << std::endl;
    std::cout << "INSTRUCTIONS:\nEnter message (50 symbols) and '->name' in the end if you want the specific client to be sent the msg." << std::endl;

    std::thread recvThread(ReceiveMessages);
    std::thread sendThread(SendMessages);

    sendThread.join();
    recvThread.join();

    closesocket(ConnectSocket);
    WSACleanup();
    return 0;
}