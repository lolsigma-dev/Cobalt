#pragma once
#include <Includes.hpp>
#include <Framework/Environment.hpp>

class CMisc {
public:
	static void InitLib(lua_State* L);
};
inline auto Misc = std::make_unique<CMisc>();
