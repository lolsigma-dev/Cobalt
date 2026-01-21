#pragma once
#include "Includes.hpp"

inline std::filesystem::path a = getenv("LOCALAPPDATA");
inline std::filesystem::path b = a / "Cobalt";
inline std::filesystem::path c = b / "workspace";

inline std::string WorkspaceDirectory() {
    if (!std::filesystem::exists(c)) {
        std::filesystem::create_directories(c);
    }
    return c.string() + "\\";
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