#pragma once
#include <Windows.h>

static uintptr_t Roblox_BASE = (uintptr_t)GetModuleHandleA(0);
#define REBASE(x) (x + Roblox_BASE)
inline const uintptr_t HYPERION = (uintptr_t)GetModuleHandle("RobloxPlayerBeta.dll");
#define HYP_REBASE(x) ((x)+HYPERION)

// ── Update only the hex values below ──

namespace Offsets {

    const uintptr_t Bitmap = HYP_REBASE(0x1C1280);
    const uintptr_t Cast_To_Varient = REBASE(0x1CE53A0);
    const uintptr_t FireMouseClick = REBASE(0x262F070);
    const uintptr_t FireMouseHoverEnter = REBASE(0x262F280);
    const uintptr_t FireMouseHoverLeave = REBASE(0x262F470);
    const uintptr_t FireProximityPrompt = REBASE(0x266F870);
    const uintptr_t FireRightMouseClick = REBASE(0x262F6E0);
    const uintptr_t FireTouchInterest = REBASE(0x2A806C0);
    const uintptr_t GetIdentityStruct = REBASE(0x47604C0);
    const uintptr_t GetLuaState = REBASE(0x1CEDE20);
    const uintptr_t GetModuleFromVMStateMap = REBASE(0x1D33260);
    const uintptr_t GetProperty = REBASE(0xC8A490);
    const uintptr_t IdentityPtr = REBASE(0x81BD8E0);
    const uintptr_t Impersonator = REBASE(0x1CE53A0);
    const uintptr_t KTable = REBASE(0x81CE200);
    const uintptr_t LuaC_step = REBASE(0x4704C60);
    const uintptr_t LuaD_throw = REBASE(0x47003E0);
    const uintptr_t LuaH_DummyNode = REBASE(0x6983298);
    const uintptr_t LuaH_getn = REBASE(0x4722360);
    const uintptr_t LuaO_NilObject = REBASE(0x69833F0);
    const uintptr_t LuaVM_Load = REBASE(0x1D1C840);
    const uintptr_t Luau_Execute = REBASE(0x47128F0);
    const uintptr_t opcodetablelookup = REBASE(0x62176B0);
    const uintptr_t Print = REBASE(0x4681920);
    const uintptr_t PushInstance1 = REBASE(0x1CF6FB0);
    const uintptr_t TaskSchedulerTargetFps = REBASE(0x81CC870);

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
            const uintptr_t FakeDMPointer = REBASE(0x81CC868);
            const uintptr_t ScriptContext = 0x3F0;
            constexpr uintptr_t PlaceId = 0x190;
            constexpr uintptr_t JobEnd = 0x1D8;
            constexpr uintptr_t JobId = 0x138;
            constexpr uintptr_t JobName = 0x18;
            constexpr uintptr_t JobStart = 0x1D0;
        }
    }
}
