#include <iostream>
#include <windows.h>

bool IsValidHandle(HANDLE h) {
    return (h != NULL) && (h != INVALID_HANDLE_VALUE);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: child_process.exe [process_number]" << std::endl;
        return 1;
    }

    int processNumber = std::atoi(argv[1]);

    HANDLE hMutex = OpenProcess(SYNCHRONIZE, FALSE, atoi(getenv("INHERITED_MUTEX_HANDLE")));

    if (!IsValidHandle(hMutex)) {
        std::cerr << "Failed to open inherited mutex handle." << std::endl;
        return 1;
    }

    std::cout << "Child process " << processNumber << " started." << std::endl;
    std::cout << "Child process " << processNumber << " completed." << std::endl;

    if (CloseHandle(hMutex) == 0) {
        std::cerr << "Failed to close inherited mutex handle. Error: " << GetLastError() << std::endl;
    }

    return 0;
}
