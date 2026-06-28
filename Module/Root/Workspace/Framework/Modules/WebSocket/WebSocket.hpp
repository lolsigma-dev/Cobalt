#pragma once
#include <Includes.hpp>
#include <Framework/Environment.hpp>

class CWebSocket {
public:
	static void InitLib(lua_State* L);
};
inline auto WebSocketLib = std::make_unique<CWebSocket>();
