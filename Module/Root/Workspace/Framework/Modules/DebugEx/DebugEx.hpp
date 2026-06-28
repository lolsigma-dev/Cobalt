#pragma once
#include <Includes.hpp>
#include <Framework/Environment.hpp>

class CDebugEx {
public:
	static void InitLib(lua_State* L);
};

inline auto DebugEx = std::make_unique<CDebugEx>();
