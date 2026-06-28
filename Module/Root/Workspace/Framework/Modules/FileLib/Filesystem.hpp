#pragma once
#include "Includes.hpp"
#include <Globals.hpp>

inline std::string DefaultWorkspaceDirectory() {
    std::filesystem::path base = getenv("LOCALAPPDATA");
    std::filesystem::path dir = base / "Cobalt" / "workspace";
    if (!std::filesystem::exists(dir)) {
        std::filesystem::create_directories(dir);
    }
    return dir.string() + "\\";
}

inline std::string WorkspaceDirectory() {
    if (!Globals::WorkspaceFolder.empty()) {
        return Globals::WorkspaceFolder;
    }
    return DefaultWorkspaceDirectory();
}

__forceinline void _SplitString(std::string Str, std::string By, std::vector<std::string>& Tokens) {
    Tokens.push_back(Str);
    const auto splitLen = By.size();
    while (true) {
        auto frag = Tokens.back();
        const auto splitAt = frag.find(By);
        if (splitAt == std::string::npos)
            break;
        Tokens.back() = frag.substr(0, splitAt);
        Tokens.push_back(frag.substr(splitAt + splitLen));
    }
}

class CFilesys {
public:
	static void InitLib(lua_State* L);
};
inline auto FileLib = std::make_unique<CFilesys>();