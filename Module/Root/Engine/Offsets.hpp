#pragma once
#include <Windows.h>

static uintptr_t Roblox_BASE = (uintptr_t)GetModuleHandleA(0);
#define REBASE(x) (x + Roblox_BASE)
inline const uintptr_t HYPERION = (uintptr_t)GetModuleHandle("RobloxPlayerBeta.dll");
#define HYP_REBASE(x) ((x)+HYPERION)

// ── Update only the hex values below ──

namespace Offsets {

    const uintptr_t Bitmap = HYP_REBASE(0x1C1280);
    const uintptr_t Cast_To_Varient = REBASE(0x1CCF0A0);
    const uintptr_t FireMouseClick = REBASE(0x2611D70);
    const uintptr_t FireMouseHoverEnter = REBASE(0x2611F80);
    const uintptr_t FireMouseHoverLeave = REBASE(0x2612170);
    const uintptr_t FireProximityPrompt = REBASE(0x2653780);
    const uintptr_t FireRightMouseClick = REBASE(0x26123E0);
    const uintptr_t FireTouchInterest = REBASE(0x2A50730);
    const uintptr_t GetIdentityStruct = REBASE(0x47166A0);
    const uintptr_t GetLuaState = REBASE(0x1CEDE20);
    const uintptr_t GetModuleFromVMStateMap = REBASE(0x1D1CF00);
    const uintptr_t GetProperty = REBASE(0xC88A20);
    const uintptr_t IdentityPtr = REBASE(0x816A140);
    const uintptr_t Impersonator = REBASE(0x1CCE250);
    const uintptr_t KTable = REBASE(0x817A980);
    const uintptr_t LuaC_step = REBASE(0x46BAC60);
    const uintptr_t LuaD_throw = REBASE(0x46B6310);
    const uintptr_t LuaH_DummyNode = REBASE(0x6933598);
    const uintptr_t LuaH_getn = REBASE(0x46D8560);
    const uintptr_t LuaO_NilObject = REBASE(0x69336F0);
    const uintptr_t LuaVM_Load = REBASE(0x1D06510);
    const uintptr_t Luau_Execute = REBASE(0x46C8A40);
    const uintptr_t opcodetablelookup = REBASE(0x61C76E0);
    const uintptr_t Print = REBASE(0x1E6B8E0);
    const uintptr_t PushInstance1 = REBASE(0x1CE0CC0);
    const uintptr_t TaskSchedulerTargetFps = REBASE(0x8179080);

    namespace External {
        namespace Bytecode {
            const uintptr_t LocalScript = 0x1A8;
            const uintptr_t ModuleScript = 0x150;
        }
        namespace UserData {
            const uintptr_t Capabilities = 0x68;
            const uintptr_t Identity = 0x48;
        }
        namespace DataModel {
            const uintptr_t GameLoaded = 0x5F8;
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
