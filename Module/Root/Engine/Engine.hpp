#pragma once
#include <Windows.h>
#include <iostream>
#include <vector>
#include "LuaVM.hpp"
#include "Encryptions.hpp"
static uintptr_t Roblox_BASE = (uintptr_t)GetModuleHandleA(0);
#define REBASE(x) (x + Roblox_BASE)
inline const uintptr_t HYPERION = (uintptr_t)GetModuleHandle("RobloxPlayerBeta.dll");
#define HYP_REBASE(x) ((x)+HYPERION)

namespace Offsets {

    const uintptr_t Print = REBASE(0x17E0070); 
    const uintptr_t PushInstance = REBASE(0x106FE10);

    /*Luau addresses*/
    const uintptr_t LuaH_DummyNode = REBASE(0x577F2A8);
    const uintptr_t Luau_Execute = REBASE(0x382DE34);
    const uintptr_t LuaD_throw = REBASE(0x3826950);
    const uintptr_t LuaO_NilObject = REBASE(0x577F8B8);
        
    const uintptr_t Bitmap = HYP_REBASE(0x1C1280); // Don't update
    const uintptr_t GetModuleFromVMStateMap = REBASE(0x1468EA0); // Don't update

    namespace Lua {
        const uintptr_t OpcodeLookupTable = REBASE(0x5c4ee20);
    }

    namespace External {

        namespace Bytecode {       /* These external members rarely change, just make sure to check every once in a while.*/
            const uintptr_t LocalScript = 0x1A8;
            const uintptr_t ModuleScript = 0x150;
        }

        namespace UserData { /*These seem to have changed*/
            const uintptr_t Capabilities = 0x68;
            const uintptr_t Identity = 0x48;    
        }

        namespace DataModel {
            const uintptr_t GameLoaded = 0x5F8;/*Make sure to check this offset as it changes every update*/
            constexpr uintptr_t Children = 0x70;
        }                                               

        namespace TaskScheduler {
            constexpr uintptr_t FakeDMtoDM = 0x1C0;
            const uintptr_t FakeDMPointer = REBASE(0x7FF0818);

            const uintptr_t ScriptContext = 0x3F0;
            constexpr uintptr_t PlaceId = 0x190;

            constexpr uintptr_t JobEnd = 0x1D8;
            constexpr uintptr_t JobId = 0x138;
            constexpr uintptr_t JobName = 0x18;
            constexpr uintptr_t JobStart = 0x1D0;
        }
    }
}
namespace ROBLOX {
    inline auto Print = (uintptr_t(__fastcall*)(int, const char*, ...))Offsets::Print;
    inline auto Luau_Execute = (void(__fastcall*)(lua_State*))Offsets::Luau_Execute;
    inline auto LuaD_Throw = (void(__fastcall*)(lua_State*, int))Offsets::LuaD_throw;
    inline auto PushInstance = (uintptr_t * (__fastcall*)(lua_State*, uintptr_t))Offsets::PushInstance;
}
