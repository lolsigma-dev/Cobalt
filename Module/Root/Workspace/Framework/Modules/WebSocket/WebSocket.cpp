#pragma once
#include "WebSocket.hpp"
#include <winhttp.h>
#include <thread>
#include <atomic>
#pragma comment(lib, "winhttp.lib")

struct WSState {
	std::string url;
	std::string host;
	std::string path;
	int port{ 443 };
	bool secure{ false };
	HINTERNET hSession{ nullptr };
	HINTERNET hConnect{ nullptr };
	HINTERNET hRequest{ nullptr };
	HINTERNET hWebSocket{ nullptr };
	std::atomic<bool> connected{ false };
	std::atomic<bool> closed{ false };
	std::string error;
};

static bool ParseWSUrl(const std::string& url, std::string& host, std::string& path, int& port, bool& secure) {
	secure = (url.find("wss://") == 0);
	if (!secure && url.find("ws://") != 0) return false;
	size_t start = secure ? 6 : 5;
	size_t colon = url.find(':', start);
	size_t slash = url.find('/', start);
	if (colon < slash) {
		host = url.substr(start, colon - start);
		port = std::stoi(url.substr(colon + 1, slash - colon - 1));
		path = (slash != std::string::npos) ? url.substr(slash) : "/";
	} else {
		host = (slash != std::string::npos) ? url.substr(start, slash - start) : url.substr(start);
		port = secure ? 443 : 80;
		path = (slash != std::string::npos) ? url.substr(slash) : "/";
	}
	if (path.empty()) path = "/";
	return true;
}

static void WSConnectThread(WSState* state) {
	state->hSession = WinHttpOpen(L"WebSocket", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, NULL, NULL, 0);
	if (!state->hSession) { state->error = "WinHttpOpen failed"; return; }

	int hostLen = MultiByteToWideChar(CP_UTF8, 0, state->host.c_str(), -1, NULL, 0);
	std::wstring wHost(hostLen, L'\0');
	MultiByteToWideChar(CP_UTF8, 0, state->host.c_str(), -1, &wHost[0], hostLen);

	int pathLen = MultiByteToWideChar(CP_UTF8, 0, state->path.c_str(), -1, NULL, 0);
	std::wstring wPath(pathLen, L'\0');
	MultiByteToWideChar(CP_UTF8, 0, state->path.c_str(), -1, &wPath[0], pathLen);

	state->hConnect = WinHttpConnect(state->hSession, wHost.c_str(), static_cast<INTERNET_PORT>(state->port), 0);
	if (!state->hConnect) { state->error = "WinHttpConnect failed"; WinHttpCloseHandle(state->hSession); state->hSession = nullptr; return; }

	DWORD flags = state->secure ? WINHTTP_FLAG_SECURE : 0;
	state->hRequest = WinHttpOpenRequest(state->hConnect, L"GET", wPath.c_str(), NULL, NULL, NULL, flags);
	if (!state->hRequest) { state->error = "WinHttpOpenRequest failed"; WinHttpCloseHandle(state->hConnect); WinHttpCloseHandle(state->hSession); state->hConnect = nullptr; state->hSession = nullptr; return; }

	DWORD wsOpt = WINHTTP_OPTION_UPGRADE_TO_WEB_SOCKET;
	if (!WinHttpSetOption(state->hRequest, WINHTTP_OPTION_UPGRADE_TO_WEB_SOCKET, NULL, 0)) {
		state->error = "Upgrade to WebSocket failed";
		WinHttpCloseHandle(state->hRequest); WinHttpCloseHandle(state->hConnect); WinHttpCloseHandle(state->hSession);
		state->hRequest = nullptr; state->hConnect = nullptr; state->hSession = nullptr;
		return;
	}

	if (!WinHttpSendRequest(state->hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0)) {
		state->error = "WinHttpSendRequest failed";
		WinHttpCloseHandle(state->hRequest); WinHttpCloseHandle(state->hConnect); WinHttpCloseHandle(state->hSession);
		state->hRequest = nullptr; state->hConnect = nullptr; state->hSession = nullptr;
		return;
	}

	if (!WinHttpReceiveResponse(state->hRequest, NULL)) {
		state->error = "WinHttpReceiveResponse failed";
		WinHttpCloseHandle(state->hRequest); WinHttpCloseHandle(state->hConnect); WinHttpCloseHandle(state->hSession);
		state->hRequest = nullptr; state->hConnect = nullptr; state->hSession = nullptr;
		return;
	}

	state->hWebSocket = WinHttpWebSocketCompleteUpgrade(state->hRequest, NULL);
	if (!state->hWebSocket) {
		state->error = "WebSocket upgrade failed";
		WinHttpCloseHandle(state->hRequest); WinHttpCloseHandle(state->hConnect); WinHttpCloseHandle(state->hSession);
		state->hRequest = nullptr; state->hConnect = nullptr; state->hSession = nullptr;
		return;
	}

	state->connected = true;
}

static int WS_gc(lua_State* L) {
	WSState** ud = reinterpret_cast<WSState**>(lua_touserdata(L, 1));
	if (ud && *ud) {
		WSState* state = *ud;
		state->closed = true;
		if (state->hWebSocket) WinHttpWebSocketClose(state->hWebSocket, WINHTTP_WEB_SOCKET_SUCCESS_CLOSE_STATUS, NULL, 0);
		if (state->hWebSocket) WinHttpCloseHandle(state->hWebSocket);
		if (state->hRequest) WinHttpCloseHandle(state->hRequest);
		if (state->hConnect) WinHttpCloseHandle(state->hConnect);
		if (state->hSession) WinHttpCloseHandle(state->hSession);
		delete state;
		*ud = nullptr;
	}
	return 0;
}

static int WS_Send(lua_State* L) {
	WSState** ud = reinterpret_cast<WSState**>(lua_touserdata(L, lua_upvalueindex(1)));
	if (!ud || !*ud) return 0;
	WSState* state = *ud;
	if (!state->connected || state->closed) return 0;
	size_t len;
	const char* data = luaL_checklstring(L, 1, &len);
	if (state->hWebSocket) {
		WinHttpWebSocketSend(state->hWebSocket, WINHTTP_WEB_SOCKET_BINARY_MESSAGE_BUFFER_TYPE, const_cast<char*>(data), static_cast<DWORD>(len));
	}
	return 0;
}

static int WS_Close(lua_State* L) {
	WSState** ud = reinterpret_cast<WSState**>(lua_touserdata(L, lua_upvalueindex(1)));
	if (!ud || !*ud) return 0;
	WSState* state = *ud;
	if (state->closed) return 0;
	state->closed = true;
	if (state->hWebSocket) {
		WinHttpWebSocketClose(state->hWebSocket, WINHTTP_WEB_SOCKET_SUCCESS_CLOSE_STATUS, NULL, 0);
		WinHttpCloseHandle(state->hWebSocket);
		state->hWebSocket = nullptr;
	}
	if (state->hRequest) { WinHttpCloseHandle(state->hRequest); state->hRequest = nullptr; }
	if (state->hConnect) { WinHttpCloseHandle(state->hConnect); state->hConnect = nullptr; }
	if (state->hSession) { WinHttpCloseHandle(state->hSession); state->hSession = nullptr; }
	return 0;
}

static int WS_connect(lua_State* L) {
	const char* urlStr = luaL_checkstring(L, 1);

	auto* state = new WSState();
	state->url = urlStr;

	if (!ParseWSUrl(state->url, state->host, state->path, state->port, state->secure)) {
		delete state;
		return luaL_error(L, "Invalid WebSocket URL: '%s'", urlStr);
	}

	// Spawn connection thread
	std::thread(WSConnectThread, state).detach();

	// Create userdata for the connection
	WSState** ud = static_cast<WSState**>(lua_newuserdata(L, sizeof(WSState*)));
	*ud = state;

	// Create metatable for GC
	if (luaL_newmetatable(L, "WebSocketConnection") == 1) {
		lua_pushcfunction(L, WS_gc);
		lua_setfield(L, -2, "__gc");
	}
	lua_setmetatable(L, -2);

	// Build connection table with methods
	lua_newtable(L);
	lua_pushvalue(L, -2);
	lua_setfield(L, -2, "__state");
	lua_newtable(L);
	lua_setmetatable(L, -2);

	// Store state ref in registry
	lua_pushvalue(L, -1);
	int ref = luaL_ref(L, LUA_REGISTRYINDEX);

	// Add Send
	lua_pushvalue(L, -2); // state userdata
	lua_pushcclosure(L, WS_Send, nullptr, 1);
	lua_setfield(L, -2, "Send");

	// Add Close
	lua_pushvalue(L, -2); // state userdata
	lua_pushcclosure(L, WS_Close, nullptr, 1);
	lua_setfield(L, -2, "Close");

	// Add OnMessage stub
	lua_pushcfunction(L, [](lua_State* L) -> int { return 0; });
	lua_setfield(L, -2, "OnMessage");

	lua_remove(L, -2); // remove userdata copy

	return 1;
}

void CWebSocket::InitLib(lua_State* L) {
	lua_newtable(L);

	lua_pushcfunction(L, WS_connect);
	lua_setfield(L, -2, "connect");

	lua_setglobal(L, "WebSocket");
}
