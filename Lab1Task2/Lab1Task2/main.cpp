#include <windows.h>
#include <vector>
#include <string>
#include <sstream>
#include <commdlg.h>
#include <shlwapi.h>

#define IDD_FONTSIZE 101
#define IDC_FONTSIZE 102

#pragma comment(lib, "shlwapi.lib")

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void StartProcess(const wchar_t* executablePath);
void CheckProcessStatus();
void DisplayProcessStatus(HDC hdc, RECT& rect);
std::wstring OpenFileDialog(HWND hwnd);
void TerminateProcessById(DWORD processId);
COLORREF ChooseColor(HWND hwnd, COLORREF currentColor);
void ChooseFontSize(HWND hwnd);

struct ProcessInfo {
    HANDLE hProcess;
    DWORD processId;
    std::wstring name;
    std::string status;
};

std::vector<ProcessInfo> processList;
COLORREF textColor = RGB(0, 0, 0);
COLORREF backgroundColor = RGB(255, 255, 255);
int textSize = 16;

void StartProcess(const wchar_t* executablePath) {
    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi;

    if (CreateProcess(executablePath, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        ProcessInfo pInfo;
        pInfo.hProcess = pi.hProcess;
        pInfo.processId = pi.dwProcessId;
        pInfo.name = PathFindFileName(executablePath);
        pInfo.status = "Running";
        processList.push_back(pInfo);

        CloseHandle(pi.hThread);
    }
    else {
        MessageBox(NULL, L"Не удалось запустить процесс", L"Ошибка", MB_OK);
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
    ss << "\n\n\n\n\n";
    for (const auto& process : processList) {
        ss << L"Имя: " << process.name << L", ID процесса: " << process.processId << L", Статус: " << process.status.c_str() << L"\n";
    }

    SetTextColor(hdc, textColor);
    SetBkMode(hdc, TRANSPARENT);
    HFONT hFont = CreateFont(textSize, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, VARIABLE_PITCH, NULL);
    SelectObject(hdc, hFont);
    DrawText(hdc, ss.str().c_str(), -1, &rect, DT_LEFT | DT_WORDBREAK | DT_NOCLIP);
    DeleteObject(hFont);
}

std::wstring OpenFileDialog(HWND hwnd) {
    OPENFILENAME ofn;
    wchar_t szFile[260] = { 0 };

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = L"Все файлы\0*.*\0";
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

COLORREF ChooseColor(HWND hwnd, COLORREF currentColor) {
    CHOOSECOLOR cc;
    COLORREF acrCustClr[16];
    ZeroMemory(&cc, sizeof(cc));
    cc.lStructSize = sizeof(cc);
    cc.hwndOwner = hwnd;
    cc.rgbResult = currentColor;
    cc.lpCustColors = acrCustClr;
    cc.Flags = CC_FULLOPEN | CC_RGBINIT;

    if (ChooseColor(&cc)) {
        return cc.rgbResult;
    }
    return currentColor;
}

void ChooseFontSize(HWND hwnd) {
    HWND hCombo = CreateWindow(L"COMBOBOX", NULL, CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE,
        10, 50, 100, 200, hwnd, NULL, NULL, NULL);
    for (int i = 10; i <= 50; ++i) {
        wchar_t buffer[3];
        swprintf(buffer, 3, L"%d", i);
        SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)buffer);
    }
    SendMessage(hCombo, CB_SETCURSEL, textSize - 10, 0);

    CreateWindow(L"BUTTON", L"OK", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        120, 50, 50, 25, hwnd, (HMENU)4, NULL, NULL);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow) {
    const wchar_t CLASS_NAME[] = L"Sample Window Class";
    WNDCLASS wc = { };

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, CLASS_NAME, L"Process Manager", WS_OVERLAPPEDWINDOW, 100, 100, 600, 600, NULL, NULL, hInstance, NULL);

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

        FillRect(hdc, &ps.rcPaint, CreateSolidBrush(backgroundColor));

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
        int lineHeight = textSize;
        int index = pt.y / lineHeight;
        if (index < processList.size()) {
            TerminateProcessById(processList[index].processId);
            InvalidateRect(hwnd, NULL, TRUE);
        }
        return 0;
    }

    case WM_SIZE: {
        InvalidateRect(hwnd, NULL, TRUE);
        return 0;
    }

    case WM_CREATE: {
        CreateWindow(L"BUTTON", L"Выбрать цвет текста", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            10, 10, 200, 30, hwnd, (HMENU)1, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
        CreateWindow(L"BUTTON", L"Выбрать цвет фона", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            220, 10, 200, 30, hwnd, (HMENU)2, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
        ChooseFontSize(hwnd);
        return 0;
    }

    case WM_COMMAND: {
        if (LOWORD(wParam) == 1) {
            textColor = ChooseColor(hwnd, textColor);
            InvalidateRect(hwnd, NULL, TRUE);
        }
        else if (LOWORD(wParam) == 2) {
            backgroundColor = ChooseColor(hwnd, backgroundColor);
            InvalidateRect(hwnd, NULL, TRUE);
        }
        else if (LOWORD(wParam) == 4) {
            HWND hCombo = FindWindowEx(hwnd, NULL, L"COMBOBOX", NULL);
            if (hCombo) {
                int index = SendMessage(hCombo, CB_GETCURSEL, 0, 0);
                if (index != CB_ERR) {
                    wchar_t buffer[3];
                    SendMessage(hCombo, CB_GETLBTEXT, index, (LPARAM)buffer);
                    textSize = _wtoi(buffer);
                    InvalidateRect(hwnd, NULL, TRUE);
                }
            }
        }
        return 0;
    }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int main() {
    HINSTANCE hInstance = GetModuleHandle(NULL);
    LPWSTR pCmdLine = GetCommandLine();
    int nCmdShow = SW_SHOWDEFAULT;

    return wWinMain(hInstance, NULL, pCmdLine, nCmdShow);
}