#include <windows.h>
#include <vector>
#include <string>
#include <sstream>
#include <commdlg.h>
#include <shlwapi.h>

#pragma comment(lib, "shlwapi.lib")

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void StartProcess(const wchar_t* executablePath);
void CheckProcessStatus();
void DisplayProcessStatus(HDC hdc, RECT& rect);
std::wstring OpenFileDialog(HWND hwnd);
void TerminateProcessById(DWORD processId);

struct ProcessInfo {
    HANDLE hProcess;
    DWORD processId;
    std::wstring name;
    std::string status;
};

std::vector<ProcessInfo> processList;

void StartProcess(const wchar_t* executablePath) {
    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi;

    if (CreateProcess(
        executablePath,
        NULL, NULL, NULL, FALSE,
        0, NULL, NULL,
        &si, &pi)) {
        
        ProcessInfo pInfo;
        pInfo.hProcess = pi.hProcess;
        pInfo.processId = pi.dwProcessId;
        pInfo.name = PathFindFileName(executablePath);
        pInfo.status = "Running";
        processList.push_back(pInfo);

        CloseHandle(pi.hThread);
    }
    else {
        MessageBox(NULL, L"Failed to start process", L"Error", MB_OK);
    }
}

void CheckProcessStatus() {
    for (auto& process : processList) {
        DWORD exitCode;
        if (process.status != "Exited") {
            if (GetExitCodeProcess(process.hProcess, &exitCode)) {
                if (exitCode != STILL_ACTIVE) {
                    process.status = "Exited";
                    CloseHandle(process.hProcess);
                }
            }
        }
    }
}

void DisplayProcessStatus(HDC hdc, RECT& rect) {
    std::wstringstream ss;
    for (const auto& process : processList) {
        ss << L"Name: " << process.name
            << L", Process ID: " << process.processId
            << L", Status: " << process.status.c_str() << L"       \n";
    }
    DrawText(hdc, ss.str().c_str(), -1, &rect, DT_LEFT | DT_TOP | DT_NOCLIP);
}

std::wstring OpenFileDialog(HWND hwnd) {
    OPENFILENAME ofn;
    wchar_t szFile[260] = { 0 };

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = L"All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileName(&ofn)) {
        return szFile;
    }
    return L"";
}

void TerminateProcessById(DWORD processId) {
    for (auto process = processList.begin(); process != processList.end(); ++process) {
        if (process->processId == processId) {
            if (process->status != "Exited") {
                TerminateProcess(process->hProcess, 0);
                CloseHandle(process->hProcess);
                process->status = "Exited";
                break;
            }
        }
    }
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow) {
    const wchar_t CLASS_NAME[] = L"Sample Window Class";
    WNDCLASS wc = { };

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0, CLASS_NAME, L"Process Manager", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL, NULL, hInstance, NULL);

    if (hwnd == NULL) {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);

    SetTimer(hwnd, 1, 1000, NULL);

    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        CheckProcessStatus();
        DisplayProcessStatus(hdc, ps.rcPaint);
        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_TIMER:
        InvalidateRect(hwnd, NULL, TRUE);
        return 0;

    case WM_LBUTTONDOWN: {
        std::wstring filePath = OpenFileDialog(hwnd);
        if (!filePath.empty()) {
            StartProcess(filePath.c_str());
            InvalidateRect(hwnd, NULL, TRUE);
        }
        return 0;
    }

    case WM_RBUTTONDOWN: {
        POINT pt = { LOWORD(lParam), HIWORD(lParam) };
        int lineHeight = 16;
        int index = pt.y / lineHeight;
        if (index < processList.size()) {
            TerminateProcessById(processList[index].processId);
            InvalidateRect(hwnd, NULL, TRUE);
        }
        return 0;
    }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int main()
{
    HINSTANCE hInstance = GetModuleHandle(NULL);
    LPWSTR pCmdLine = GetCommandLine();
    int nCmdShow = SW_SHOWDEFAULT;

    return wWinMain(hInstance, NULL, pCmdLine, nCmdShow);
}