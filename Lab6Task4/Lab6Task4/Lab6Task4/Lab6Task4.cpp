#define _WIN32_DCOM

#include <windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <wbemidl.h>
#include <comdef.h>
#include <winioctl.h>
#include <setupapi.h>
#include <devguid.h>
#include <regstr.h>
#include <iphlpapi.h>
#include <iomanip>
#include <tlhelp32.h>

#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "iphlpapi.lib")

#pragma warning(disable : 4996)

std::string GetOSVersion() {
    OSVERSIONINFO osvi;
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&osvi);
    return "Windows " + std::to_string(osvi.dwMajorVersion) + "." +
        std::to_string(osvi.dwMinorVersion) + " Build " +
        std::to_string(osvi.dwBuildNumber);
}

std::string GetMachineName() {
    char buffer[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD size = sizeof(buffer);
    GetComputerNameA(buffer, &size);
    return std::string(buffer);
}

std::string GetUserName() {
    char buffer[256];
    DWORD size = sizeof(buffer);
    GetUserNameA(buffer, &size);
    return std::string(buffer);
}

std::string GetProcessorName() {
    char buffer[256];
    DWORD size = sizeof(buffer);
    HKEY hKey;
    std::string processorName;

    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        if (RegQueryValueExA(hKey, "ProcessorNameString", NULL, NULL, (LPBYTE)buffer, &size) == ERROR_SUCCESS) {
            processorName = std::string(buffer);
        }
        RegCloseKey(hKey);
    }

    return processorName;
}

std::string GetProcessorFrequency() {
    HKEY hKey;
    DWORD frequency = 0;
    DWORD size = sizeof(frequency);

    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        RegQueryValueExA(hKey, "~MHz", NULL, NULL, (LPBYTE)&frequency, &size);
        RegCloseKey(hKey);
    }

    return std::to_string(frequency);
}

std::string GetCacheInfo() {
    DWORD bufferSize = 0;
    GetLogicalProcessorInformation(nullptr, &bufferSize);

    std::vector<SYSTEM_LOGICAL_PROCESSOR_INFORMATION> buffer(bufferSize / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION));
    std::ostringstream info;

    if (!GetLogicalProcessorInformation(buffer.data(), &bufferSize)) {
        info << "Ошибка получения информации о процессоре\n";
        return info.str();
    }
    size_t l1Cache = 0, l2Cache = 0, l3Cache = 0;

    for (const auto& info : buffer) {
        if (info.Relationship == RelationCache) {
            switch (info.Cache.Level) {
            case 1: l1Cache += info.Cache.Size; break;
            case 2: l2Cache += info.Cache.Size; break;
            case 3: l3Cache += info.Cache.Size; break;
            default: break;
            }
        }
    }

    info << "Размер L1 кеша: " << l1Cache / 1024 << " KБ\n"
        << "Размер L2 кеша: " << l2Cache / 1024 << " KБ\n"
        << "Размер L3 кеша: " << l3Cache / 1024 << " KБ\n";

    return info.str();
}

std::string GetProcessorInfo() {
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);

    std::ostringstream info;
    info << "Количество логических процессоров: " << sysInfo.dwNumberOfProcessors << "\n";
    info << "Количество ядер: " << (sysInfo.dwNumberOfProcessors / 2) << "\n";
    info << "Частота процессора: " << GetProcessorFrequency() << " МГц\n";
    info << GetCacheInfo();
    return info.str();
}

std::string GetMemoryManufacturer() {
    HRESULT hres;

    hres = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hres)) {
        return "Ошибка инициализации COM";
    }

    hres = CoInitializeSecurity(
        NULL,
        -1,                          
        NULL,                        
        NULL,                        
        RPC_C_AUTHN_LEVEL_DEFAULT, 
        RPC_C_IMP_LEVEL_IMPERSONATE, 
        NULL,                        
        EOAC_NONE,                 
        NULL                        
    );

    if (FAILED(hres)) {
        CoUninitialize();
        return "Ошибка инициализации безопасности COM";
    }

    IWbemLocator* pLoc = NULL;

    hres = CoCreateInstance(
        CLSID_WbemLocator,
        0,
        CLSCTX_INPROC_SERVER,
        IID_IWbemLocator, (LPVOID*)&pLoc);

    if (FAILED(hres)) {
        CoUninitialize();
        return "Ошибка создания локатора WMI";
    }

    IWbemServices* pSvc = NULL;

    hres = pLoc->ConnectServer(
        _bstr_t(L"ROOT\\CIMV2"), 
        NULL,                    
        NULL,                    
        0,                       
        NULL,                    
        0,                       
        0,                       
        &pSvc                    
    );

    if (FAILED(hres)) {
        pLoc->Release();
        CoUninitialize();
        return "Ошибка подключения к WMI";
    }

    IEnumWbemClassObject* pEnumerator = NULL;
    hres = pSvc->ExecQuery(
        bstr_t("WQL"),
        bstr_t("SELECT Manufacturer FROM Win32_PhysicalMemory"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL,
        &pEnumerator);

    if (FAILED(hres)) {
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return "Ошибка выполнения запроса WMI";
    }

    IWbemClassObject* pclsObj = NULL;
    ULONG uReturn = 0;
    std::string manufacturer;

    while (pEnumerator) {
        HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
        if (0 == uReturn) {
            break;
        }

        VARIANT vtProp;
        hr = pclsObj->Get(L"Manufacturer", 0, &vtProp, 0, 0);
        if (SUCCEEDED(hr)) {
            manufacturer = _com_util::ConvertBSTRToString(vtProp.bstrVal);
            VariantClear(&vtProp);
        }
        pclsObj->Release();
    }

    pSvc->Release();
    pLoc->Release();
    pEnumerator->Release();
    CoUninitialize();

    return manufacturer.empty() ? "Производитель не найден" : manufacturer;
}

std::string GetMemoryInfo() {
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memInfo);

    std::ostringstream info;
    info << "Общий объем оперативной памяти: " << memInfo.ullTotalPhys / (1024 * 1024) << " МБ\n";
    info << "Доступно: " << memInfo.ullAvailPhys / (1024 * 1024) << " МБ\n";
    info << "Объем виртуальной памяти: " << memInfo.ullTotalVirtual / (1024 * 1024) << " МБ\n";
    info << "Доступно виртуальной памяти: " << memInfo.ullAvailVirtual / (1024 * 1024) << " МБ\n";
    info << "Производитель: " << GetMemoryManufacturer() << "\n";

    return info.str();
}

std::string GetGraphicsCardInfo() {
    HRESULT hres;

    hres = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hres)) {
        return "Не удалось инициализировать библиотеку COM.";
    }

    hres = CoInitializeSecurity(
        NULL,
        -1,
        NULL,
        NULL,
        RPC_C_AUTHN_LEVEL_DEFAULT,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL,
        EOAC_NONE,
        NULL
    );

    if (FAILED(hres)) {
        CoUninitialize();
        return "Не удалось инициализировать безопасность.";
    }

    IWbemLocator* pLoc = NULL;

    hres = CoCreateInstance(
        CLSID_WbemLocator,
        0,
        CLSCTX_INPROC_SERVER,
        IID_IWbemLocator, (LPVOID*)&pLoc);

    if (FAILED(hres)) {
        CoUninitialize();
        return "Не удалось создать объект IWbemLocator.";
    }

    IWbemServices* pSvc = NULL;

    hres = pLoc->ConnectServer(
        _bstr_t(L"ROOT\\CIMV2"),
        NULL, NULL, 0,
        NULL, 0, 0, &pSvc);

    if (FAILED(hres)) {
        pLoc->Release();
        CoUninitialize();
        return "Не удалось подключиться к WMI.";
    }

    IEnumWbemClassObject* pEnumerator = NULL;
    hres = pSvc->ExecQuery(
        bstr_t("WQL"),
        bstr_t("SELECT Name FROM Win32_VideoController"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL,
        &pEnumerator);

    if (FAILED(hres)) {
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return "Запрос на получение видеоконтроллера не удался.";
    }

    IWbemClassObject* pclsObj = NULL;
    ULONG uReturn = 0;
    std::ostringstream info;

    while (pEnumerator) {
        HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
        if (0 == uReturn) break;

        VARIANT vtProp;
        VariantInit(&vtProp);

        hr = pclsObj->Get(L"Name", 0, &vtProp, 0, 0);
        info << "Видеокарта: " << _com_util::ConvertBSTRToString(vtProp.bstrVal) << "\n";

        VariantClear(&vtProp);
        pclsObj->Release();
    }

    pSvc->Release();
    pLoc->Release();
    pEnumerator->Release();
    CoUninitialize();

    return info.str().empty() ? "Видеокарта не найдена." : info.str();
}

std::string GetNetworkInfo() {
    ULONG bufferSize = 0;
    GetAdaptersInfo(NULL, &bufferSize);
    PIP_ADAPTER_INFO pAdapterInfo = (IP_ADAPTER_INFO*)malloc(bufferSize);
    std::ostringstream networkInfo;

    if (GetAdaptersInfo(pAdapterInfo, &bufferSize) == NO_ERROR) {
        PIP_ADAPTER_INFO pAdapter = pAdapterInfo;
        networkInfo << "Сетевые параметры:\n";
        while (pAdapter) {
            networkInfo << "Имя адаптера: " << pAdapter->AdapterName << "\n";
            networkInfo << "MAC-адрес: ";
            for (UINT i = 0; i < pAdapter->AddressLength; ++i) {
                if (i != 0) networkInfo << "-";
                networkInfo << std::hex << std::setw(2) << std::setfill('0') << (int)pAdapter->Address[i];
            }
            networkInfo << "\n";
            networkInfo << "IP-адрес: " << pAdapter->IpAddressList.IpAddress.String << "\n";
            networkInfo << "Описание: " << pAdapter->Description << "\n\n";
            pAdapter = pAdapter->Next;
        }
    }
    else {
        networkInfo << "Не удалось получить информацию о сетевых адаптерах.";
    }

    free(pAdapterInfo);
    return networkInfo.str();
}

void PrintProcessesInfo() {
    PROCESSENTRY32 pe32;
    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to take snapshot of processes." << std::endl;
        return;
    }

    pe32.dwSize = sizeof(PROCESSENTRY32);
    if (!Process32First(hProcessSnap, &pe32)) {
        std::cerr << "Failed to get first process." << std::endl;
        CloseHandle(hProcessSnap);
        return;
    }

    std::cout << "Запущенные процессы:" << std::endl;
    do {
        std::wcout << "Process: " << pe32.szExeFile << std::endl;
    } while (Process32Next(hProcessSnap, &pe32));

    CloseHandle(hProcessSnap);
}

void PrintSystemInfo() {
    
    std::cout << "Процессор: " << GetProcessorName() << std::endl;
    std::cout << GetProcessorInfo() << std::endl;
    std::cout << GetMemoryInfo() << std::endl;
    std::cout << GetGraphicsCardInfo() << std::endl;
    std::cout << "Система: " << GetOSVersion() << std::endl;
    std::cout << "Имя компьютера: " << GetMachineName() << std::endl;
    std::cout << "Имя пользователя: " << GetUserName() << std::endl << std::endl;
    std::cout << GetNetworkInfo() << std::endl;
    PrintProcessesInfo();
}

int main() {
    setlocale(LC_ALL, "Russian");
    PrintSystemInfo();
    return 0;
}