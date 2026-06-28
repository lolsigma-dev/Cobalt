#pragma once	
#include "Script.hpp"
#include <Bytecode/Bytecode.hpp>

int identifyexecutor(lua_State* L) {
	lua_pushstring(L, "Cobalt");
	lua_pushstring(L, "1.0.0");
	return 2;
}

int getscriptbytecode(lua_State* L) {
	if (lua_type(L, 1) != LUA_TUSERDATA) { lua_pushnil(L); return 1; }
	uintptr_t script = *(uintptr_t*)lua_touserdata(L, 1);
	if (!script) { lua_pushnil(L); return 1; }
	lua_getfield(L, 1, "ClassName");
	const char* name = lua_tostring(L, -1);
	lua_pop(L, 1);
	uintptr_t addr = name && strcmp(name, "ModuleScript") == 0
		? *(uintptr_t*)(script + ModuleScriptOffset)
		: *(uintptr_t*)(script + LocalScriptOffset);
	std::string bytecode = ReadBytecode(addr);
	std::string code = addr ? DecompressBytecode(bytecode) : "";
	if (code.empty()) { lua_pushnil(L); return 1; }
	lua_pushlstring(L, code.data(), code.size());
	return 1;
}

static int getscripthash(lua_State* L) {
	if (lua_type(L, 1) != LUA_TUSERDATA) { lua_pushnil(L); return 1; }
	lua_getfield(L, 1, "Source");
	if (lua_isstring(L, -1)) {
		size_t len;
		const char* src = lua_tolstring(L, -1, &len);
		unsigned char hash[16];
		memset(hash, 0, sizeof(hash));
		for (size_t i = 0; i < len; i++)
			hash[i % 16] ^= src[i];
		char hex[33];
		for (int i = 0; i < 16; i++)
			sprintf(hex + i * 2, "%02x", hash[i]);
		hex[32] = '\0';
		lua_pushstring(L, hex);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

static int getcallingscript(lua_State* L) {
	lua_pushnil(L);
	return 1;
}

static int getscriptclosure(lua_State* L) {
	if (lua_type(L, 1) != LUA_TUSERDATA) { lua_pushnil(L); return 1; }
	lua_getfield(L, 1, "Source");
	if (lua_isstring(L, -1)) {
		const char* src = lua_tostring(L, -1);
		std::string bytecode = Execution->Compile(src);
		if (luau_load(L, "@Cobalt", bytecode.data(), bytecode.size(), 0) != LUA_OK) {
			lua_pop(L, 1);
			lua_pushnil(L);
			return 1;
		}
		lua_pushvalue(L, -1);
		return 1;
	}
	lua_pushnil(L);
	return 1;
}

void CScript::InitLib(lua_State* L) {
	declare__Global(L, "identifyexecutor", identifyexecutor);
	declare__Global(L, "getexecutorname", identifyexecutor);
	declare__Global(L, "getscriptbytecode", getscriptbytecode);
	declare__Global(L, "dumpstring", getscriptbytecode);
	declare__Global(L, "getscripthash", getscripthash);
	declare__Global(L, "getcallingscript", getcallingscript);
	declare__Global(L, "getscriptclosure", getscriptclosure);
	declare__Global(L, "getscriptfunction", getscriptclosure);
}
