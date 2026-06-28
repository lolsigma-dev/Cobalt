#pragma once
#include "Closures.hpp"
#include <lfunc.h>

int loadstring(lua_State* l) {
    lua_check(l, 2);
    luaL_checktype(l, 1, LUA_TSTRING);
    auto source = lua_tostring(l, 1);
    auto chunk_name = luaL_optstring(l, 2, "@Cobalt");
    auto bytecode = Execution->Compile(source);
    if (luau_load(l, chunk_name, bytecode.data(), bytecode.size(), 0) != LUA_OK) {
        lua_pushnil(l);
        lua_pushvalue(l, -2);
        return 2;
    }
    if (Closure* func = lua_toclosure(l, -1))
        if (func->l.p)
            Scheduler->SetProtoCaps(func->l.p, &Capabilities);
    lua_setsafeenv(l, LUA_GLOBALSINDEX, false);
    return 1;
}

static int iscclosure(lua_State* L) {
    luaL_checktype(L, 1, LUA_TFUNCTION);
    Closure* closure = clvalue(luaA_toobject(L, 1));
    lua_pushboolean(L, closure && closure->isC);
    return 1;
}

static int islclosure(lua_State* L) {
    luaL_checktype(L, 1, LUA_TFUNCTION);
    Closure* closure = clvalue(luaA_toobject(L, 1));
    lua_pushboolean(L, closure && !closure->isC);
    return 1;
}

static int hookfunction(lua_State* L) {
    luaL_checktype(L, 1, LUA_TFUNCTION);
    luaL_checktype(L, 2, LUA_TFUNCTION);
    Closure* target = clvalue(luaA_toobject(L, 1));
    lua_pushvalue(L, 1);
    lua_pushcclosure(L, lua_tocfunction(L, 2), nullptr, 0);
    Closure* hook = clvalue(luaA_toobject(L, -1));
    if (target && hook) {
        hook->isC = target->isC;
        hook->stacksize = target->stacksize;
        if (target->isC) {
            hook->c.f = target->c.f;
        }
    }
    lua_pushvalue(L, 1);
    return 1;
}

void CClosures::InitLib(lua_State* L) {
    declare__Global(L, "loadstring", loadstring);
    declare__Global(L, "iscclosure", iscclosure);
    declare__Global(L, "islclosure", islclosure);
    declare__Global(L, "hookfunction", hookfunction);
    declare__Global(L, "replaceclosure", hookfunction);
}
