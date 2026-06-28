#pragma once
#include <Includes.hpp>
#include <Framework/Environment.hpp>

class CConsole {
public:
	static void InitLib(lua_State* L);
};
inline auto Console = std::make_unique<CConsole>();
