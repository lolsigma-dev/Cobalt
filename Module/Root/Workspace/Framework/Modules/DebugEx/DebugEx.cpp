#pragma once
#include "DebugEx.hpp"
#include <lobject.h>
#include <lfunc.h>
#include <lapi.h>
#include <lstate.h>
#include <lstring.h>
#include <ltable.h>

static int debug_getinfo(lua_State* L) {
	luaL_checktype(L, 1, LUA_TFUNCTION);
	Closure* cl = clvalue(luaA_toobject(L, 1));
	lua_newtable(L);
	if (cl) {
		if (cl->isC) {
			lua_pushstring(L, "C");
			lua_setfield(L, -2, "what");
			lua_pushstring(L, "[C]");
			lua_setfield(L, -2, "source");
			lua_pushstring(L, "[C]");
			lua_setfield(L, -2, "short_src");
			lua_pushinteger(L, -1);
			lua_setfield(L, -2, "currentline");
			lua_pushinteger(L, 0);
			lua_setfield(L, -2, "nups");
			lua_pushinteger(L, cl->nupvalues);
			lua_setfield(L, -2, "numparams");
			lua_pushinteger(L, 0);
			lua_setfield(L, -2, "is_vararg");
			lua_pushinteger(L, 1);
			lua_setfield(L, -2, "is_vararg");
		} else {
			Proto* p = cl->l.p;
			lua_pushstring(L, "Lua");
			lua_setfield(L, -2, "what");
			lua_pushstring(L, p->source ? getstr(p->source) : "@Cobalt");
			lua_setfield(L, -2, "source");
			lua_pushstring(L, p->source ? getstr(p->source) : "@Cobalt");
			lua_setfield(L, -2, "short_src");
			lua_pushinteger(L, p->linedefined);
			lua_setfield(L, -2, "linedefined");
			lua_pushinteger(L, p->lastlinedefined);
			lua_setfield(L, -2, "lastlinedefined");
			lua_pushinteger(L, p->numparams);
			lua_setfield(L, -2, "numparams");
			lua_pushinteger(L, p->is_vararg);
			lua_setfield(L, -2, "is_vararg");
			lua_pushinteger(L, p->sizep);
			lua_setfield(L, -2, "nups");
		}
		lua_pushvalue(L, 1);
		lua_setfield(L, -2, "func");
		lua_pushstring(L, cl->isC ? "C closure" : "Lua closure");
		lua_setfield(L, -2, "name");
	}
	return 1;
}

static int debug_getconstant(lua_State* L) {
	luaL_checktype(L, 1, LUA_TFUNCTION);
	int idx = (int)luaL_checkinteger(L, 2);
	Closure* cl = clvalue(luaA_toobject(L, 1));
	if (!cl || cl->isC || !cl->l.p || idx < 1 || idx > (int)cl->l.p->sizek) {
		lua_pushnil(L);
		return 1;
	}
	TValue* k = &cl->l.p->k[idx - 1];
	switch (k->tt) {
	case LUA_TSTRING:
		lua_pushstring(L, getstr(tsvalue(k)));
		break;
	case LUA_TBOOLEAN:
		lua_pushboolean(L, k->value.b);
		break;
	case LUA_TNUMBER:
		lua_pushnumber(L, k->value.n);
		break;
	case LUA_TNIL:
	default:
		lua_pushnil(L);
		break;
	}
	return 1;
}

static int debug_getconstants(lua_State* L) {
	luaL_checktype(L, 1, LUA_TFUNCTION);
	Closure* cl = clvalue(luaA_toobject(L, 1));
	if (!cl || cl->isC || !cl->l.p) {
		lua_newtable(L);
		return 1;
	}
	Proto* p = cl->l.p;
	lua_createtable(L, p->sizek, 0);
	for (int i = 0; i < (int)p->sizek; i++) {
		TValue* k = &p->k[i];
		switch (k->tt) {
		case LUA_TSTRING:
			lua_pushstring(L, getstr(tsvalue(k)));
			break;
		case LUA_TBOOLEAN:
			lua_pushboolean(L, k->value.b);
			break;
		case LUA_TNUMBER:
			lua_pushnumber(L, k->value.n);
			break;
		default:
			lua_pushnil(L);
			break;
		}
		lua_rawseti(L, -2, i + 1);
	}
	return 1;
}

static int debug_getstack(lua_State* L) {
	int level = (int)luaL_checkinteger(L, 1);
	lua_Debug ar;
	if (lua_getstack(L, level, &ar)) {
		lua_getinfo(L, "Sl", &ar);
		if (lua_gettop(L) >= 2 && !lua_isnone(L, 2)) {
			int idx = (int)lua_tointeger(L, 2);
			const char* val = lua_tostring(L, -1);
			lua_pushstring(L, val ? val : "");
			return 1;
		} else {
			lua_newtable(L);
			for (int i = 1;; i++) {
				const char* name = lua_getlocal(L, &ar, i);
				if (!name) break;
				lua_setfield(L, -2, name);
			}
			return 1;
		}
	}
	lua_pushnil(L);
	return 1;
}

static int debug_setconstant(lua_State* L) {
	luaL_checktype(L, 1, LUA_TFUNCTION);
	int idx = (int)luaL_checkinteger(L, 2);
	Closure* cl = clvalue(luaA_toobject(L, 1));
	if (!cl || cl->isC || !cl->l.p || idx < 1 || idx > (int)cl->l.p->sizek)
		return 0;
	TValue* k = &cl->l.p->k[idx - 1];
	switch (lua_type(L, 3)) {
	case LUA_TSTRING:
		setstrvalue(k, luaS_new(L, lua_tostring(L, 3)));
		break;
	case LUA_TBOOLEAN:
		setbvalue(k, lua_toboolean(L, 3));
		break;
	case LUA_TNUMBER:
		setnvalue(k, lua_tonumber(L, 3));
		break;
	default:
		setnilvalue(k);
		break;
	}
	return 0;
}

static int debug_setstack(lua_State* L) {
	int level = (int)luaL_checkinteger(L, 1);
	int idx = (int)luaL_checkinteger(L, 2);
	lua_Debug ar;
	if (lua_getstack(L, level, &ar)) {
		lua_getinfo(L, "Sl", &ar);
		const char* name = lua_getlocal(L, &ar, idx);
		if (name) {
			lua_pushvalue(L, 3);
			lua_setlocal(L, &ar, idx);
		}
	}
	return 0;
}

static int debug_getupvalue(lua_State* L) {
	luaL_checktype(L, 1, LUA_TFUNCTION);
	int idx = (int)luaL_checkinteger(L, 2);
	Closure* cl = clvalue(luaA_toobject(L, 1));
	if (!cl || idx < 1 || idx > (int)cl->nupvalues) {
		lua_pushnil(L);
		return 1;
	}
	TValue* up = &cl->uprefs[idx - 1];
	switch (up->tt) {
	case LUA_TSTRING:
		lua_pushstring(L, getstr(tsvalue(up)));
		break;
	case LUA_TBOOLEAN:
		lua_pushboolean(L, up->value.b);
		break;
	case LUA_TNUMBER:
		lua_pushnumber(L, up->value.n);
		break;
	case LUA_TFUNCTION:
		lua_pushcfunction(L, (lua_CFunction)up->value.gc);
		break;
	default:
		lua_pushnil(L);
		break;
	}
	return 1;
}

static int debug_getupvalues(lua_State* L) {
	luaL_checktype(L, 1, LUA_TFUNCTION);
	Closure* cl = clvalue(luaA_toobject(L, 1));
	if (!cl || cl->nupvalues == 0) {
		lua_newtable(L);
		return 1;
	}
	lua_createtable(L, (int)cl->nupvalues, 0);
	for (int i = 0; i < (int)cl->nupvalues; i++) {
		TValue* up = &cl->uprefs[i];
		switch (up->tt) {
		case LUA_TSTRING:
			lua_pushstring(L, getstr(tsvalue(up)));
			break;
		case LUA_TBOOLEAN:
			lua_pushboolean(L, up->value.b);
			break;
		case LUA_TNUMBER:
			lua_pushnumber(L, up->value.n);
			break;
		case LUA_TFUNCTION:
			lua_pushcfunction(L, (lua_CFunction)up->value.gc);
			break;
		default:
			lua_pushnil(L);
			break;
		}
		lua_rawseti(L, -2, i + 1);
	}
	return 1;
}

static int debug_setupvalue(lua_State* L) {
	luaL_checktype(L, 1, LUA_TFUNCTION);
	int idx = (int)luaL_checkinteger(L, 2);
	Closure* cl = clvalue(luaA_toobject(L, 1));
	if (!cl || idx < 1 || idx > (int)cl->nupvalues) {
		lua_pushnil(L);
		return 1;
	}
	TValue* up = &cl->uprefs[idx - 1];
	lua_pushvalue(L, 3);
	switch (lua_type(L, 3)) {
	case LUA_TSTRING:
		setstrvalue(up, luaS_new(L, lua_tostring(L, 3)));
		break;
	case LUA_TBOOLEAN:
		setbvalue(up, lua_toboolean(L, 3));
		break;
	case LUA_TNUMBER:
		setnvalue(up, lua_tonumber(L, 3));
		break;
	case LUA_TNIL:
		setnilvalue(up);
		break;
	default:
		break;
	}
	lua_pushvalue(L, 1);
	return 1;
}

static int debug_getproto(lua_State* L) {
	luaL_checktype(L, 1, LUA_TFUNCTION);
	int idx = (int)luaL_checkinteger(L, 2);
	bool active = lua_toboolean(L, 3);
	Closure* cl = clvalue(luaA_toobject(L, 1));
	if (!cl || cl->isC || !cl->l.p || idx < 1 || idx > (int)cl->l.p->sizep) {
		lua_pushnil(L);
		return 1;
	}
	Proto* p = cl->l.p->p[idx - 1];
	if (active) {
		lua_createtable(L, (int)p->sizep > 0 ? (int)p->sizep : 1, 0);
		Closure* newCl = luaF_newLclosure(L, 0, L->global->mt[LUA_TFUNCTION], p);
		if (newCl) {
			setclvalue(L, L->top, newCl);
			incr_top(L);
			lua_rawseti(L, -2, 1);
		}
	} else {
		lua_pushlightuserdata(L, p);
	}
	return 1;
}

static int debug_getprotos(lua_State* L) {
	luaL_checktype(L, 1, LUA_TFUNCTION);
	Closure* cl = clvalue(luaA_toobject(L, 1));
	if (!cl || cl->isC || !cl->l.p || cl->l.p->sizep == 0) {
		lua_newtable(L);
		return 1;
	}
	Proto* p = cl->l.p;
	lua_createtable(L, (int)p->sizep, 0);
	for (int i = 0; i < (int)p->sizep; i++) {
		Proto* inner = p->p[i];
		Closure* newCl = luaF_newLclosure(L, 0, L->global->mt[LUA_TFUNCTION], inner);
		if (newCl) {
			setclvalue(L, L->top, newCl);
			incr_top(L);
		} else {
			lua_pushnil(L);
		}
		lua_rawseti(L, -2, i + 1);
	}
	return 1;
}

void CDebugEx::InitLib(lua_State* L) {
	lua_getglobal(L, "debug");
	if (lua_istable(L, -1)) {
		lua_pushcfunction(L, debug_getinfo);
		lua_setfield(L, -2, "getinfo");
		lua_pushcfunction(L, debug_getconstant);
		lua_setfield(L, -2, "getconstant");
		lua_pushcfunction(L, debug_getconstants);
		lua_setfield(L, -2, "getconstants");
		lua_pushcfunction(L, debug_getstack);
		lua_setfield(L, -2, "getstack");
		lua_pushcfunction(L, debug_setconstant);
		lua_setfield(L, -2, "setconstant");
		lua_pushcfunction(L, debug_setstack);
		lua_setfield(L, -2, "setstack");
		lua_pushcfunction(L, debug_getupvalue);
		lua_setfield(L, -2, "getupvalue");
		lua_pushcfunction(L, debug_getupvalues);
		lua_setfield(L, -2, "getupvalues");
		lua_pushcfunction(L, debug_setupvalue);
		lua_setfield(L, -2, "setupvalue");
		lua_pushcfunction(L, debug_getproto);
		lua_setfield(L, -2, "getproto");
		lua_pushcfunction(L, debug_getprotos);
		lua_setfield(L, -2, "getprotos");
	}
	lua_pop(L, 1);
}
