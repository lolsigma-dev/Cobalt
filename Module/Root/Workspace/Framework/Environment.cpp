#include "Environment.hpp"

void CEnvironment::Initialize(lua_State* L) {
	CScript::InitLib(L);
	CHTTP::InitLib(L);
	CFilesys::InitLib(L);
	CInput::InitLib(L);
	CClosures::InitLib(L);
	CMisc::InitLib(L);
	CCrypt::InitLib(L);
	CConsole::InitLib(L);
	CDebugEx::InitLib(L);
	CDrawing::InitLib(L);
	CWebSocket::InitLib(L);

	lua_newtable(L);
	lua_setglobal(L, "_G");

	lua_newtable(L);
	lua_setglobal(L, "shared");
}
