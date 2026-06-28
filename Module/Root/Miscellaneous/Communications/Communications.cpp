#pragma once
#define WIN32_LEAN_AND_MEAN
#include "Communications.hpp"
#include <string>
#include <vector>
#include <thread>
#include <windows.h>
#include <cryptopp/base64.h>
#include <cryptopp/filters.h>

constexpr const char* VELOCITY_PIPE_PREFIX = "\\\\.\\pipe\\uoQcySKXSUxxJNpVQyatpHQwYoGfhcbh_";

inline void(__fastcall* luaprintf)(std::int32_t, const char*, ...) =
    reinterpret_cast<void(__fastcall*)(std::int32_t, const char*, ...)>(ROBLOX::Print);

static std::string Base64Decode(const std::string& encoded) {
    std::string decoded;
    try {
        CryptoPP::StringSource ss(encoded, true,
            new CryptoPP::Base64Decoder(
                new CryptoPP::StringSink(decoded)
            )
        );
    } catch (...) {
        return {};
    }
    return decoded;
}

void NamedPipeServer() {
    DWORD pid = GetCurrentProcessId();
    std::string pipeName = VELOCITY_PIPE_PREFIX + std::to_string(pid);

    luaprintf(1, "[COBALT->VELOCITY] Pipe: %s", pipeName.c_str());

    while (true) {
        HANDLE hPipe = CreateNamedPipeA(
            pipeName.c_str(),
            PIPE_ACCESS_INBOUND,
            PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
            PIPE_UNLIMITED_INSTANCES,
            0,
            0x100000,
            0,
            nullptr
        );

        if (hPipe == INVALID_HANDLE_VALUE) {
            Sleep(100);
            continue;
        }

        if (!ConnectNamedPipe(hPipe, nullptr)) {
            if (GetLastError() != ERROR_PIPE_CONNECTED) {
                CloseHandle(hPipe);
                continue;
            }
        }

        std::string fullData;
        char tempBuf[4096];
        DWORD bytesRead = 0;

        while (ReadFile(hPipe, tempBuf, sizeof(tempBuf), &bytesRead, nullptr) && bytesRead > 0) {
            fullData.append(tempBuf, bytesRead);
        }

        if (!fullData.empty()) {
            size_t end = fullData.find_last_not_of("\r\n\0");
            if (end != std::string::npos)
                fullData.erase(end + 1);

            std::string decoded = Base64Decode(fullData);

            if (!decoded.empty()) {
                const std::string wsPrefix = "setworkspacefolder: ";
                if (decoded.find(wsPrefix) == 0) {
                    Globals::WorkspaceFolder = decoded.substr(wsPrefix.length());
                    std::replace(Globals::WorkspaceFolder.begin(), Globals::WorkspaceFolder.end(), '\\', '/');
                    while (!Globals::WorkspaceFolder.empty() && Globals::WorkspaceFolder.back() == '/')
                        Globals::WorkspaceFolder.pop_back();
                    Globals::WorkspaceFolder += '/';
                    luaprintf(1, "[COBALT->VELOCITY] Workspace set: %s", Globals::WorkspaceFolder.c_str());
                } else {
                    if (Globals::ExecutorThread) {
                        Execution->SendScript(Globals::ExecutorThread, decoded);
                    } else {
                        luaprintf(3, "[COBALT->VELOCITY] No valid lua_State");
                    }
                }
            }
        }

        DisconnectNamedPipe(hPipe);
        CloseHandle(hPipe);
    }
}

void CCommunications::Initialize() {
    std::thread(NamedPipeServer).detach();
}
