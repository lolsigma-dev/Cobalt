#pragma once
#include "Console.hpp"

static HWND g_ConsoleHWND = nullptr;
static HANDLE g_ConsoleOut = nullptr;
static HANDLE g_ConsoleIn = nullptr;

static int rconsolecreate(lua_State* L) {
	if (!g_ConsoleOut) {
		AllocConsole();
		g_ConsoleOut = GetStdHandle(STD_OUTPUT_HANDLE);
		g_ConsoleIn = GetStdHandle(STD_INPUT_HANDLE);
		g_ConsoleHWND = GetConsoleWindow();
		SetConsoleTitleA("Cobalt Console");
	}
	return 0;
}

static int rconsoledestroy(lua_State* L) {
	if (g_ConsoleOut) {
		FreeConsole();
		g_ConsoleOut = nullptr;
		g_ConsoleIn = nullptr;
		g_ConsoleHWND = nullptr;
	}
	return 0;
}

static int rconsoleprint(lua_State* L) {
	if (!g_ConsoleOut) rconsolecreate(L);
	if (g_ConsoleOut) {
		const char* text = luaL_checkstring(L, 1);
		DWORD written;
		WriteConsoleA(g_ConsoleOut, text, (DWORD)strlen(text), &written, nullptr);
		WriteConsoleA(g_ConsoleOut, "\r\n", 2, &written, nullptr);
	}
	return 0;
}

static int rconsoleclear(lua_State* L) {
	if (g_ConsoleOut) {
		COORD topLeft = { 0, 0 };
		CONSOLE_SCREEN_BUFFER_INFO screen;
		DWORD written;
		GetConsoleScreenBufferInfo(g_ConsoleOut, &screen);
		FillConsoleOutputCharacterA(g_ConsoleOut, ' ', screen.dwSize.X * screen.dwSize.Y, topLeft, &written);
		SetConsoleCursorPosition(g_ConsoleOut, topLeft);
	}
	return 0;
}

static int rconsoleinput(lua_State* L) {
	if (!g_ConsoleOut) rconsolecreate(L);
	if (g_ConsoleIn) {
		char buf[1024];
		DWORD read;
		if (ReadConsoleA(g_ConsoleIn, buf, sizeof(buf) - 1, &read, nullptr)) {
			buf[read] = '\0';
			size_t len = strlen(buf);
			while (len > 0 && (buf[len - 1] == '\n' || buf[len - 1] == '\r'))
				buf[--len] = '\0';
			lua_pushstring(L, buf);
			return 1;
		}
	}
	lua_pushnil(L);
	return 1;
}

static int rconsolesettitle(lua_State* L) {
	const char* title = luaL_checkstring(L, 1);
	SetConsoleTitleA(title);
	return 0;
}

void CConsole::InitLib(lua_State* L) {
	declare__Global(L, "rconsolecreate", rconsolecreate);
	declare__Global(L, "consolecreate", rconsolecreate);
	declare__Global(L, "rconsoledestroy", rconsoledestroy);
	declare__Global(L, "consoledestroy", rconsoledestroy);
	declare__Global(L, "rconsoleprint", rconsoleprint);
	declare__Global(L, "consoleprint", rconsoleprint);
	declare__Global(L, "rconsoleclear", rconsoleclear);
	declare__Global(L, "consoleclear", rconsoleclear);
	declare__Global(L, "rconsoleinput", rconsoleinput);
	declare__Global(L, "consoleinput", rconsoleinput);
	declare__Global(L, "rconsolesettitle", rconsolesettitle);
	declare__Global(L, "rconsolename", rconsolesettitle);
	declare__Global(L, "consolesettitle", rconsolesettitle);
}
