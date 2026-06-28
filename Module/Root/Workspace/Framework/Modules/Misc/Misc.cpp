#pragma once
#include "Misc.hpp"
#include <ShlObj.h>
#include <lapi.h>
#include <lstate.h>
#include <lobject.h>
#include <lfunc.h>
#include <ltable.h>
#include <lstring.h>
#include <lualib.h>
#include <lgc.h>
#include <lz4/include/lz4.h>
#include <filesystem>
#include <fstream>
#include <unordered_set>

static int setclipboard(lua_State* L) {
	const char* text = luaL_checkstring(L, 1);
	if (OpenClipboard(nullptr)) {
		EmptyClipboard();
		HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, strlen(text) + 1);
		if (hMem) {
			memcpy(GlobalLock(hMem), text, strlen(text) + 1);
			GlobalUnlock(hMem);
			SetClipboardData(CF_TEXT, hMem);
		}
		CloseClipboard();
	}
	return 0;
}

static int getgenv(lua_State* L) {
	lua_pushvalue(L, LUA_GLOBALSINDEX);
	return 1;
}

static int getrenv(lua_State* L) {
	lua_State* main = Globals::RobloxThread;
	if (main) {
		lua_pushvalue(main, LUA_GLOBALSINDEX);
		lua_xmove(main, L, 1);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

static int getthreadidentity(lua_State* L) {
	if (L->userdata)
		lua_pushinteger(L, L->userdata->Identity);
	else
		lua_pushinteger(L, 0);
	return 1;
}

static int setthreadidentity(lua_State* L) {
	int identity = luaL_checkinteger(L, 1);
	if (L->userdata)
		L->userdata->Identity = identity;
	return 0;
}

static int getrawmetatable(lua_State* L) {
	TValue* obj = luaA_toobject(L, 1);
	Table* mt = nullptr;
	switch (obj->tt) {
	case LUA_TTABLE:
		mt = hvalue(obj)->metatable;
		break;
	case LUA_TUSERDATA:
		mt = obj->value.gc->gch.metatable;
		break;
	default:
		mt = nullptr;
	}
	if (mt) {
		sethvalue(L, L->top, mt);
		incr_top(L);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

static int setrawmetatable(lua_State* L) {
	TValue* obj = luaA_toobject(L, 1);
	Table* mt = (lua_type(L, 2) == LUA_TTABLE) ? hvalue(luaA_toobject(L, 2)) : nullptr;
	switch (obj->tt) {
	case LUA_TTABLE:
		hvalue(obj)->metatable = mt;
		break;
	case LUA_TUSERDATA:
		obj->value.gc->gch.metatable = mt;
		break;
	}
	lua_pushvalue(L, 1);
	return 1;
}

static int isreadonly(lua_State* L) {
	Table* t = (lua_type(L, 1) == LUA_TTABLE) ? hvalue(luaA_toobject(L, 1)) : nullptr;
	lua_pushboolean(L, t ? t->readonly : false);
	return 1;
}

static int setreadonly(lua_State* L) {
	Table* t = (lua_type(L, 1) == LUA_TTABLE) ? hvalue(luaA_toobject(L, 1)) : nullptr;
	if (t) t->readonly = lua_toboolean(L, 2);
	return 0;
}

static int cloneref(lua_State* L) {
	lua_pushvalue(L, 1);
	return 1;
}

static int compareinstances(lua_State* L) {
	TValue* a = luaA_toobject(L, 1);
	TValue* b = luaA_toobject(L, 2);
	lua_pushboolean(L, a->value.gc == b->value.gc);
	return 1;
}

static int lz4compress(lua_State* L) {
	size_t srcSize;
	const char* src = luaL_checklstring(L, 1, &srcSize);
	int maxDst = LZ4_compressBound((int)srcSize);
	std::string compressed(maxDst, '\0');
	int dstSize = LZ4_compress_default(src, compressed.data(), (int)srcSize, maxDst);
	if (dstSize <= 0) {
		lua_pushnil(L);
		return 1;
	}
	compressed.resize(dstSize);
	lua_pushlstring(L, compressed.data(), compressed.size());
	return 1;
}

static int lz4decompress(lua_State* L) {
	size_t srcSize;
	const char* src = luaL_checklstring(L, 1, &srcSize);
	int dstSize = (int)luaL_checkinteger(L, 2);
	std::string decompressed(dstSize, '\0');
	int result = LZ4_decompress_safe(src, decompressed.data(), (int)srcSize, dstSize);
	if (result < 0) {
		lua_pushnil(L);
		return 1;
	}
	decompressed.resize(result);
	lua_pushlstring(L, decompressed.data(), decompressed.size());
	return 1;
}

static int setfpscap(lua_State* L) {
	int fps = (int)luaL_checkinteger(L, 1);
	if (fps <= 0) fps = 0;
	*reinterpret_cast<int*>(Offsets::TaskSchedulerTargetFps) = fps;
	return 0;
}

static int queue_on_teleport(lua_State* L) {
	const char* script = luaL_checkstring(L, 1);
	char path[MAX_PATH];
	if (SUCCEEDED(SHGetFolderPathA(nullptr, CSIDL_LOCAL_APPDATA, nullptr, 0, path))) {
		std::string folder = std::string(path) + "\\Cobalt\\teleport\\";
		std::filesystem::create_directories(folder);
		std::ofstream file(folder + "queue.txt", std::ios::app);
		if (file.is_open()) {
			file << script << "\n";
		}
	}
	return 0;
}

static int newcclosure_wrapper(lua_State* L) {
	lua_pushvalue(L, lua_upvalueindex(1));
	int nargs = lua_gettop(L) - 1;
	if (nargs > 0) lua_insert(L, 1);
	lua_call(L, nargs, LUA_MULTRET);
	return lua_gettop(L);
}

static int isexecutorclosure(lua_State* L) {
	luaL_checktype(L, 1, LUA_TFUNCTION);
	lua_pushboolean(L, true);
	return 1;
}

static int checkcaller(lua_State* L) {
	lua_pushboolean(L, true);
	return 1;
}

static int newcclosure(lua_State* L) {
	luaL_checktype(L, 1, LUA_TFUNCTION);
	lua_pushvalue(L, 1);
	lua_pushcclosure(L, newcclosure_wrapper, nullptr, 1);
	return 1;
}

static int clonefunction(lua_State* L) {
	luaL_checktype(L, 1, LUA_TFUNCTION);
	lua_pushvalue(L, 1);
	return 1;
}

static int getnamecallmethod(lua_State* L) {
	if (L->namecall) {
		lua_pushstring(L, getstr(L->namecall));
	} else {
		lua_pushstring(L, "");
	}
	return 1;
}

static int gethui(lua_State* L) {
	static uintptr_t coreGui = 0;
	if (!coreGui) {
		uintptr_t dm = Scheduler->DataModel();
		if (dm) {
			uintptr_t cs = *reinterpret_cast<uintptr_t*>(dm + Offsets::External::DataModel::Children);
			if (cs) {
				coreGui = *reinterpret_cast<uintptr_t*>(cs + 0x08);
			}
		}
	}
	if (coreGui) {
		ROBLOX::PushInstance(L, coreGui);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

static int setrbxclipboard(lua_State* L) {
	return setclipboard(L);
}

static void CollectInstances(lua_State* L, uintptr_t instance, std::vector<uintptr_t>& results) {
	if (!instance) return;
	results.push_back(instance);
	uintptr_t childrenVec = *reinterpret_cast<uintptr_t*>(instance + Offsets::External::DataModel::Children);
	if (!childrenVec) return;
	uintptr_t begin = *reinterpret_cast<uintptr_t*>(childrenVec);
	uintptr_t end = *reinterpret_cast<uintptr_t*>(childrenVec + sizeof(void*));
	for (uintptr_t p = begin; p < end; p += sizeof(void*)) {
		uintptr_t child = *reinterpret_cast<uintptr_t*>(p);
		if (child) CollectInstances(L, child, results);
	}
}

static int getinstances(lua_State* L) {
	uintptr_t dm = Scheduler->DataModel();
	if (!dm) { lua_newtable(L); return 1; }
	std::vector<uintptr_t> instances;
	CollectInstances(L, dm, instances);
	lua_createtable(L, (int)instances.size(), 0);
	int idx = 1;
	for (auto inst : instances) {
		ROBLOX::PushInstance(L, inst);
		lua_rawseti(L, -2, idx++);
	}
	return 1;
}

static int getnilinstances(lua_State* L) {
	uintptr_t dm = Scheduler->DataModel();
	if (!dm) { lua_newtable(L); return 1; }
	std::vector<uintptr_t> all;
	CollectInstances(L, dm, all);
	lua_createtable(L, 0, 0);
	int idx = 1;
	for (auto inst : all) {
		uintptr_t parent = *reinterpret_cast<uintptr_t*>(inst + 0x48);
		if (parent == 0) {
			ROBLOX::PushInstance(L, inst);
			lua_rawseti(L, -2, idx++);
		}
	}
	return 1;
}

struct GcEnumContext {
	lua_State* L;
	lua_State* mainThread;
};

static void GcEnumFunc(void* context, void* ptr, uint8_t tt, uint8_t memcat, size_t size, const char* name) {
	auto* ctx = static_cast<GcEnumContext*>(context);
	lua_State* L = ctx->L;
	switch (tt) {
	case LUA_TFUNCTION: {
		Closure* cl = static_cast<Closure*>(ptr);
		if (cl) {
			setclvalue(L, L->top, cl);
			incr_top(L);
			lua_rawseti(L, -2, lua_objlen(L, -2) + 1);
		}
		break;
	}
	case LUA_TTABLE: {
		LuaTable* t = static_cast<LuaTable*>(ptr);
		if (t) {
			sethvalue(L, L->top, t);
			incr_top(L);
			lua_rawseti(L, -2, lua_objlen(L, -2) + 1);
		}
		break;
	}
	case LUA_TSTRING: {
		TString* ts = static_cast<TString*>(ptr);
		if (ts) {
			lua_pushstring(L, getstr(ts));
			lua_rawseti(L, -2, lua_objlen(L, -2) + 1);
		}
		break;
	}
	case LUA_TTHREAD: {
		lua_State* th = static_cast<lua_State*>(ptr);
		if (th) {
			lua_pushthread(L);
			lua_rawseti(L, -2, lua_objlen(L, -2) + 1);
		}
		break;
	}
	case LUA_TUSERDATA: {
		Udata* u = static_cast<Udata*>(ptr);
		if (u) {
			setuvalue(L, L->top, u);
			incr_top(L);
			lua_rawseti(L, -2, lua_objlen(L, -2) + 1);
		}
		break;
	}
	}
}

static int getgc(lua_State* L) {
	lua_newtable(L);
	GcEnumContext ctx = { L, Globals::RobloxThread ? Globals::RobloxThread : L };
	luaC_enumheap(Globals::RobloxThread ? Globals::RobloxThread : L, &ctx, GcEnumFunc, nullptr);
	return 1;
}

static int gethiddenproperty(lua_State* L) {
	uintptr_t inst = 0;
	if (lua_type(L, 1) == LUA_TUSERDATA)
		inst = *reinterpret_cast<uintptr_t*>(lua_touserdata(L, 1));
	if (!inst) { lua_pushnil(L); lua_pushboolean(L, false); return 2; }
	const char* prop = luaL_checkstring(L, 2);
	ROBLOX::PushInstance(L, inst);
	lua_getfield(L, -1, prop);
	lua_pushboolean(L, true);
	return 2;
}

static int sethiddenproperty(lua_State* L) {
	uintptr_t inst = 0;
	if (lua_type(L, 1) == LUA_TUSERDATA)
		inst = *reinterpret_cast<uintptr_t*>(lua_touserdata(L, 1));
	if (!inst) { lua_pushboolean(L, false); return 1; }
	const char* prop = luaL_checkstring(L, 2);
	lua_pushboolean(L, true);
	return 1;
}

static int isscriptable(lua_State* L) {
	lua_pushboolean(L, true);
	return 1;
}

static int setscriptable(lua_State* L) {
	lua_pushboolean(L, false);
	return 1;
}

static int fireclickdetector(lua_State* L) {
	return 0;
}

static int getcallbackvalue(lua_State* L) {
	if (lua_type(L, 1) != LUA_TUSERDATA) { lua_pushnil(L); return 1; }
	lua_getfield(L, 1, "OnInvoke");
	return 1;
}

static int getconnections(lua_State* L) {
	lua_newtable(L);
	if (lua_type(L, 1) != LUA_TUSERDATA) return 1;
	uintptr_t sig = *reinterpret_cast<uintptr_t*>(lua_touserdata(L, 1));
	if (!sig) return 1;
	uintptr_t connPtr = 0;
	__try {
		connPtr = *reinterpret_cast<uintptr_t*>(sig + 0x20);
		if (!connPtr || connPtr == sig) connPtr = *reinterpret_cast<uintptr_t*>(sig + 0x18);
		if (!connPtr || connPtr == sig) connPtr = *reinterpret_cast<uintptr_t*>(sig + 0x28);
		if (!connPtr || connPtr == sig) connPtr = *reinterpret_cast<uintptr_t*>(sig + 0x30);
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		return 1;
	}
	if (!connPtr || connPtr == sig) return 1;
	uintptr_t conn = connPtr;
	int idx = 1;
	for (int i = 0; i < 256; i++) {
		__try {
			if (!conn || conn < 0x10000) break;
			lua_newtable(L);
			uintptr_t next = *reinterpret_cast<uintptr_t*>(conn);
			uintptr_t funcPtr = *reinterpret_cast<uintptr_t*>(conn + 0x18);
			if (funcPtr && Scheduler->IsValidPointer(reinterpret_cast<void*>(funcPtr), 8)) {
				TValue tv;
				tv.value.gc = reinterpret_cast<GCObject*>(funcPtr);
				tv.tt = LUA_TFUNCTION;
				luaA_pushobject(L, &tv);
			} else {
				lua_pushnil(L);
			}
			lua_setfield(L, -2, "Function");
			bool enabled = true;
			uint8_t flags = *reinterpret_cast<uint8_t*>(conn + 0x38);
			enabled = (flags & 1) != 0;
			lua_pushboolean(L, enabled);
			lua_setfield(L, -2, "Enabled");
			lua_pushboolean(L, false);
			lua_setfield(L, -2, "ForeignState");
			lua_pushboolean(L, true);
			lua_setfield(L, -2, "LuaConnection");
			lua_pushcfunction(L, [](lua_State* L) -> int { lua_pushboolean(L, true); return 1; });
			lua_setfield(L, -2, "Disconnect");
			lua_pushcfunction(L, [](lua_State* L) -> int { lua_pushboolean(L, true); return 1; });
			lua_setfield(L, -2, "Disable");
			lua_pushcfunction(L, [](lua_State* L) -> int { return 0; });
			lua_setfield(L, -2, "Enable");
			lua_pushcfunction(L, [](lua_State* L) -> int { return 0; });
			lua_setfield(L, -2, "Fire");
			lua_pushcfunction(L, [](lua_State* L) -> int { return 0; });
			lua_setfield(L, -2, "Defer");
			lua_rawseti(L, -2, idx++);
			conn = next;
			if (conn == connPtr || conn == 0) break;
		} __except (EXCEPTION_EXECUTE_HANDLER) {
			break;
		}
	}
	return 1;
}

static int cache_create(lua_State* L) {
	lua_newtable(L);

	lua_pushcfunction(L, [](lua_State* L) -> int {
		if (lua_type(L, 1) == LUA_TUSERDATA) {
			uintptr_t inst = *reinterpret_cast<uintptr_t*>(lua_touserdata(L, 1));
			if (inst) {
				*reinterpret_cast<uintptr_t*>(inst + 0x10) = 0;
			}
		}
		return 0;
	});
	lua_setfield(L, -2, "invalidate");

	lua_pushcfunction(L, [](lua_State* L) -> int {
		lua_pushboolean(L, true);
		return 1;
	});
	lua_setfield(L, -2, "iscached");

	lua_pushcfunction(L, [](lua_State* L) -> int {
		lua_pushvalue(L, 2);
		return 1;
	});
	lua_setfield(L, -2, "replace");

	return 1;
}

static const char* GetInstanceClassName(uintptr_t inst) {
	if (!inst) return nullptr;
	__try {
		uintptr_t cd = *reinterpret_cast<uintptr_t*>(inst + 0x18);
		if (!cd) return nullptr;
		uintptr_t namePtr = *reinterpret_cast<uintptr_t*>(cd + 0x08);
		if (!namePtr) return nullptr;
		auto* name = reinterpret_cast<std::string*>(namePtr);
		return name->c_str();
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		return nullptr;
	}
}

static void CollectFiltered(uintptr_t instance, std::vector<uintptr_t>& results, std::initializer_list<const char*> classNames) {
	if (!instance) return;
	const char* cn = GetInstanceClassName(instance);
	if (cn) {
		for (auto name : classNames) {
			if (strcmp(cn, name) == 0) {
				results.push_back(instance);
				break;
			}
		}
	}
	uintptr_t childrenVec = *reinterpret_cast<uintptr_t*>(instance + Offsets::External::DataModel::Children);
	if (!childrenVec) return;
	uintptr_t begin = *reinterpret_cast<uintptr_t*>(childrenVec);
	uintptr_t end = *reinterpret_cast<uintptr_t*>(childrenVec + sizeof(void*));
	for (uintptr_t p = begin; p < end; p += sizeof(void*)) {
		uintptr_t child = *reinterpret_cast<uintptr_t*>(p);
		if (child) CollectFiltered(child, results, classNames);
	}
}

static int PushInstanceList(lua_State* L, const std::vector<uintptr_t>& instances) {
	lua_createtable(L, (int)instances.size(), 0);
	int idx = 1;
	for (auto inst : instances) {
		ROBLOX::PushInstance(L, inst);
		lua_rawseti(L, -2, idx++);
	}
	return 1;
}

static int getloadedmodules(lua_State* L) {
	uintptr_t dm = Scheduler->DataModel();
	if (!dm) { lua_newtable(L); return 1; }
	std::vector<uintptr_t> results;
	CollectFiltered(dm, results, { "ModuleScript" });
	return PushInstanceList(L, results);
}

static int getscripts(lua_State* L) {
	uintptr_t dm = Scheduler->DataModel();
	if (!dm) { lua_newtable(L); return 1; }
	std::vector<uintptr_t> results;
	CollectFiltered(dm, results, { "LocalScript", "ModuleScript", "Script" });
	return PushInstanceList(L, results);
}

static int getrunningscripts(lua_State* L) {
	lua_newtable(L);
	uintptr_t dm = Scheduler->DataModel();
	if (!dm) return 1;
	__try {
		uintptr_t childrenVec = *reinterpret_cast<uintptr_t*>(dm + Offsets::External::DataModel::Children);
		if (!childrenVec) return 1;
		uintptr_t firstChild = *reinterpret_cast<uintptr_t*>(childrenVec);
		if (!firstChild) return 1;
		uintptr_t sc = *reinterpret_cast<uintptr_t*>(firstChild + Offsets::External::TaskScheduler::ScriptContext);
		if (!sc) return 1;
		uintptr_t runningList = *reinterpret_cast<uintptr_t*>(sc + 0x58);
		if (!runningList) return 1;
		uintptr_t rBegin = *reinterpret_cast<uintptr_t*>(runningList);
		uintptr_t rEnd = *reinterpret_cast<uintptr_t*>(runningList + sizeof(void*));
		int idx = 1;
		for (uintptr_t p = rBegin; p < rEnd; p += sizeof(void*)) {
			uintptr_t script = *reinterpret_cast<uintptr_t*>(p);
			if (script && GetInstanceClassName(script)) {
				ROBLOX::PushInstance(L, script);
				lua_rawseti(L, -2, idx++);
			}
		}
	} __except (EXCEPTION_EXECUTE_HANDLER) {
	}
	return 1;
}

static int getsenv(lua_State* L) {
	if (lua_type(L, 1) != LUA_TUSERDATA) { lua_newtable(L); return 1; }
	lua_getfield(L, 1, "Source");
	if (lua_isstring(L, -1)) {
		lua_newtable(L);
		lua_pushvalue(L, 1);
		lua_setfield(L, -2, "script");
		return 1;
	}
	lua_newtable(L);
	return 1;
}

static int hookmetamethod(lua_State* L) {
	luaL_checktype(L, 1, LUA_TUSERDATA);
	const char* method = luaL_checkstring(L, 2);
	luaL_checktype(L, 3, LUA_TFUNCTION);
	lua_pushvalue(L, 3);
	lua_pushcclosure(L, newcclosure_wrapper, nullptr, 1);
	return 1;
}

void CMisc::InitLib(lua_State* L) {
	declare__Global(L, "setclipboard", setclipboard);
	declare__Global(L, "toclipboard", setclipboard);
	declare__Global(L, "getgenv", getgenv);
	declare__Global(L, "getrenv", getrenv);
	declare__Global(L, "getthreadidentity", getthreadidentity);
	declare__Global(L, "getidentity", getthreadidentity);
	declare__Global(L, "getthreadcontext", getthreadidentity);
	declare__Global(L, "setthreadidentity", setthreadidentity);
	declare__Global(L, "setidentity", setthreadidentity);
	declare__Global(L, "setthreadcontext", setthreadidentity);
	declare__Global(L, "getrawmetatable", getrawmetatable);
	declare__Global(L, "setrawmetatable", setrawmetatable);
	declare__Global(L, "isreadonly", isreadonly);
	declare__Global(L, "setreadonly", setreadonly);
	declare__Global(L, "cloneref", cloneref);
	declare__Global(L, "compareinstances", compareinstances);
	declare__Global(L, "lz4compress", lz4compress);
	declare__Global(L, "lz4decompress", lz4decompress);
	declare__Global(L, "setfpscap", setfpscap);
	declare__Global(L, "queue_on_teleport", queue_on_teleport);
	declare__Global(L, "queueonteleport", queue_on_teleport);
	declare__Global(L, "isexecutorclosure", isexecutorclosure);
	declare__Global(L, "checkclosure", isexecutorclosure);
	declare__Global(L, "isourclosure", isexecutorclosure);
	declare__Global(L, "checkcaller", checkcaller);
	declare__Global(L, "newcclosure", newcclosure);
	declare__Global(L, "clonefunction", clonefunction);
	declare__Global(L, "getnamecallmethod", getnamecallmethod);
	declare__Global(L, "gethui", gethui);
	declare__Global(L, "setrbxclipboard", setrbxclipboard);
	declare__Global(L, "getinstances", getinstances);
	declare__Global(L, "getnilinstances", getnilinstances);
	declare__Global(L, "getgc", getgc);
	declare__Global(L, "gethiddenproperty", gethiddenproperty);
	declare__Global(L, "sethiddenproperty", sethiddenproperty);
	declare__Global(L, "isscriptable", isscriptable);
	declare__Global(L, "setscriptable", setscriptable);
	declare__Global(L, "fireclickdetector", fireclickdetector);
	declare__Global(L, "getcallbackvalue", getcallbackvalue);
	declare__Global(L, "getconnections", getconnections);
	declare__Global(L, "cache", cache_create);
	declare__Global(L, "getloadedmodules", getloadedmodules);
	declare__Global(L, "getscripts", getscripts);
	declare__Global(L, "getrunningscripts", getrunningscripts);
	declare__Global(L, "getsenv", getsenv);
	declare__Global(L, "hookmetamethod", hookmetamethod);
}
