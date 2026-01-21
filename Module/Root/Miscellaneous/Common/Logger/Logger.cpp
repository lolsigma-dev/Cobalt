#include "Logger.hpp"

HANDLE hPipe = INVALID_HANDLE_VALUE;

void Logger::Init() {
    std::thread([] {
        while (true) {
            hPipe = CreateFileA(
                R"(\\.\pipe\LoggerInformationPipe)",
                GENERIC_WRITE,
                0,
                nullptr,
                OPEN_EXISTING,
                0,
                nullptr
            );

            if (hPipe != INVALID_HANDLE_VALUE) {
   
                break;
            }

            DWORD error = GetLastError();
            if (error == ERROR_PIPE_BUSY) {
                if (!WaitNamedPipeA((R"(\\.\pipe\LoggerInformationPipe)"), 5000)) {
         
                    continue;
                }
            }
            else {

                break;
            }
        }
        }).detach();
}

void Logger::printf(const char* fmt, ...) {
    if (hPipe == INVALID_HANDLE_VALUE) {
  
        return;
    }

    char buffer[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    strcat_s(buffer, "\n");

    DWORD bytesWritten = 0;
    BOOL result = WriteFile(
        hPipe,
        buffer,
        (DWORD)strlen(buffer),
        &bytesWritten,
        nullptr
    );

    if (!result || bytesWritten == 0) {
        DWORD error = GetLastError();

    }
    else {

    }
}
