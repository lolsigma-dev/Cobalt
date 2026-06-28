#pragma once
#include <Includes.hpp>
#include <Framework/Environment.hpp>

class CCrypt {
public:
	static void InitLib(lua_State* L);
};
inline auto Crypt = std::make_unique<CCrypt>();
