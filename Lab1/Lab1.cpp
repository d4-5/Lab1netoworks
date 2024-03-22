#include <windows.h>
#include <iostream>
#include <vector>
#include <thread>

HANDLE hMutex;
HANDLE hSemaphore;
std::vector<HANDLE> hProcesses;

void CreateAndStartProcesses();
void CheckProcessCompletion();
void MonitorProcesses(HANDLE hProcess);

int main() {
    hMutex = CreateMutex(NULL, FALSE, "lab1-mutex");
    if (hMutex == NULL) {
        std::cerr << "Failed to create named mutex. Error: " << GetLastError() << std::endl;
        return 1;
    }

    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        std::cerr << "Another instance is already running." << std::endl;
        return 1;
    }

    hSemaphore = CreateSemaphore(NULL, 3, 3, "MySemaphore");
    if (hSemaphore == NULL) {
        std::cerr << "Failed to create semaphore. Error: " << GetLastError() << std::endl;
        CloseHandle(hMutex);
        return 1;
    }

    CreateAndStartProcesses();

    Sleep(5000);

    CheckProcessCompletion();

    Sleep(20000);

    CloseHandle(hSemaphore);
    CloseHandle(hMutex);
    return 0;
}

void CreateAndStartProcesses() {
    SECURITY_ATTRIBUTES sa;
    ZeroMemory(&sa, sizeof(sa));
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;
    HANDLE hIMutex = CreateMutex(&sa, FALSE, NULL);

    for (int i = 0; i < 10; ++i) {
        WaitForSingleObject(hSemaphore, INFINITE);

        PROCESS_INFORMATION pi;
        STARTUPINFO si = { sizeof(si) };
        TCHAR szCommandLine[MAX_PATH];
        _snprintf_s(szCommandLine, MAX_PATH, "child_process.exe %d", i);

        BOOL bInheritHandles = TRUE;

        BOOL bSuccess = CreateProcess(NULL, szCommandLine, NULL, NULL, bInheritHandles, 0, NULL, NULL, &si, &pi);

        if (bSuccess) {
            hProcesses.push_back(pi.hProcess);
            std::thread monitorThread(MonitorProcesses, pi.hProcess);
            monitorThread.detach();
        }
        else {
            std::cerr << "Failed to create child process. Error: " << GetLastError() << std::endl;
            ReleaseSemaphore(hSemaphore, 1, NULL);
        }
    }
}

void CheckProcessCompletion() {
    for (size_t i = 0; i < hProcesses.size(); ++i) {
        DWORD dwExitCode;
        if (GetExitCodeProcess(hProcesses[i], &dwExitCode) && dwExitCode == STILL_ACTIVE) {
            std::cout << "Process " << i << " with PID " << GetProcessId(hProcesses[i]) << " is still running." << std::endl;
        }
        else {
            std::cout << "Process " << i << " with PID " << GetProcessId(hProcesses[i]) << " has completed." << std::endl;
        }
    }
}

void MonitorProcesses(HANDLE hProcess) {
    WaitForSingleObject(hProcess, INFINITE);
    CloseHandle(hProcess);
    ReleaseSemaphore(hSemaphore, 1, NULL);
}
