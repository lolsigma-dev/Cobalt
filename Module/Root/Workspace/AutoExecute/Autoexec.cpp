//
// Created by @binninwl_ on 15/05/2025.
//
#pragma once
#include "Autoexec.hpp"
#include <Windows.h>
#include <ShlObj.h>
#include <filesystem>
#include <fstream>
#include <string>
#include <thread>
#include <atomic>
#include <chrono>

std::atomic<bool> autoexecStarted(false);

void CAutoExecute::Run() {
    using namespace std::chrono_literals;
    Logger::printf("[COBALT - UTILITY FUNCTIONS] Started AutoExecution");

    auto start = std::chrono::steady_clock::now();

    while (!autoexecStarted.load()) {
        auto now = std::chrono::steady_clock::now();
        if (now - start >= 2s) {
            char path[MAX_PATH];
            std::string folderPath;
            if (SUCCEEDED(SHGetFolderPathA(nullptr, CSIDL_LOCAL_APPDATA, nullptr, 0, path))) {
                folderPath = std::string(path) + "\\Cobalt\\autoexec\\";
            }

            if (!folderPath.empty()) {
                namespace fs = std::filesystem;
                try {
                    if (fs::exists(folderPath)) {
                        for (const auto& entry : fs::directory_iterator(folderPath)) {
                            if (!entry.is_regular_file())
                                continue;

                            auto ext = entry.path().extension().string();
                            if (ext != ".lua" && ext != ".txt")
                                continue;

                            std::ifstream fileStream(entry.path(), std::ios::in | std::ios::binary);
                            if (!fileStream)
                                continue;

                            std::string script((std::istreambuf_iterator<char>(fileStream)),
                                std::istreambuf_iterator<char>());

                            lua_State* L = Globals::RobloxThread;
                            if (!L) {
                                ROBLOX::Print(3, "[COBALT->AUTOEXECUTION] No valid luastate to execute in!");
                                return;
                            }

                            CExecution::SendScript(L, script); 
                        }
                    }
                }
                catch (const std::exception&) {
                    // silent fail because syfm
                }
            }

            autoexecStarted.store(true);
            break;
        }
        std::this_thread::sleep_for(10ms);
    }
}
