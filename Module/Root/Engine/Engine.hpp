#pragma once
#include <Windows.h>
#include <iostream>
#include <vector>
#include "LuaVM.hpp"
#include "Encryptions.hpp"
#include "Offsets.hpp"

namespace ROBLOX {
    inline auto Print = (uintptr_t(__fastcall*)(int, const char*, ...))Offsets::Print;
    inline auto Luau_Execute = (void(__fastcall*)(lua_State*))Offsets::Luau_Execute;
    inline auto LuaD_Throw = (void(__fastcall*)(lua_State*, int))Offsets::LuaD_throw;
    inline auto PushInstance = (uintptr_t * (__fastcall*)(lua_State*, uintptr_t))Offsets::PushInstance1;
}
