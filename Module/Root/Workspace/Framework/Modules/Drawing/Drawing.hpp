#pragma once
#include <Includes.hpp>
#include <Framework/Environment.hpp>

class CDrawing {
public:
	static void InitLib(lua_State* L);
};
inline auto Drawing = std::make_unique<CDrawing>();
