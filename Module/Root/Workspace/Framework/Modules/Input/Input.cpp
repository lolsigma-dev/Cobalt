//
// Created by @binninwl_ on 30/05/2025.
//
#pragma once 
#include "Input.hpp"

bool RobloxActive() {
    return (GetForegroundWindow() == FindWindowA(NULL, "Roblox"));
};

 int isrbxactive(lua_State* L)
{
    lua_pushboolean(L, RobloxActive());
    return 1;
};

 int keypress(lua_State* L) {
    luaL_checktype(L, 1, LUA_TNUMBER);
    UINT key = lua_tointeger(L, 1);

    if (RobloxActive())
        keybd_event(0, (BYTE)MapVirtualKeyA(key, MAPVK_VK_TO_VSC), KEYEVENTF_SCANCODE, 0);

    return 0;
};

std::int32_t keytap(lua_State* L) {
    luaL_checktype(L, 1, LUA_TNUMBER);
    UINT key = lua_tointeger(L, 1);

    if (!RobloxActive())
        return 0;

    keybd_event(0, MapVirtualKeyA(key, MAPVK_VK_TO_VSC), KEYEVENTF_SCANCODE, 0);
    keybd_event(0, MapVirtualKeyA(key, MAPVK_VK_TO_VSC), KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP, 0);

    return 0;
};

 int keyrelease(lua_State* L) {
    luaL_checktype(L, 1, LUA_TNUMBER);
    UINT key = lua_tointeger(L, 1);

    if (RobloxActive())
        keybd_event(0, (BYTE)MapVirtualKeyA(key, MAPVK_VK_TO_VSC), KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP, 0);

    return 0;
};

int mouse1click(lua_State* L) {
    if (RobloxActive())
        mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);

    return 0;
};

int mouse1press(lua_State* L) {
    if (RobloxActive())
        mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);

    return 0;
};

int mouse1release(lua_State* L) {
    if (RobloxActive())
        mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);

    return 0;
};

int mouse2click(lua_State* L) {
    if (RobloxActive())
        mouse_event(MOUSEEVENTF_RIGHTDOWN | MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);

    return 0;
};

int mouse2press(lua_State* L)
{
    if (RobloxActive())
        mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, 0);

    return 0;
};

int mouse2release(lua_State* L) {
    if (RobloxActive())
        mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);

    return 0;
};

int mousemoveabs(lua_State* L) {
    luaL_checktype(L, 1, LUA_TNUMBER);
    luaL_checktype(L, 2, LUA_TNUMBER);

    int x = lua_tointeger(L, 1);
    int y = lua_tointeger(L, 2);

    if (!RobloxActive())
        return 0;

    int width = GetSystemMetrics(SM_CXSCREEN) - 1;
    int height = GetSystemMetrics(SM_CYSCREEN) - 1;

    RECT CRect;
    GetClientRect(GetForegroundWindow(), &CRect);

    POINT Point{ CRect.left, CRect.top };
    ClientToScreen(GetForegroundWindow(), &Point);

    x = (x + Point.x) * (65535 / width);
    y = (y + Point.y) * (65535 / height);

    mouse_event(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE, x, y, 0, 0);
    return 0;
};

int mousemoverel(lua_State* L) {
    luaL_checktype(L, 1, LUA_TNUMBER);
    luaL_checktype(L, 2, LUA_TNUMBER);

    int x = lua_tointeger(L, 1);
    int y = lua_tointeger(L, 2);

    if (RobloxActive())
        mouse_event(MOUSEEVENTF_MOVE, x, y, 0, 0);

    return 0;
};

int mousescroll(lua_State* L) {
    luaL_checktype(L, 1, LUA_TNUMBER);

    int amt = lua_tointeger(L, 1);

    if (RobloxActive())
        mouse_event(MOUSEEVENTF_WHEEL, 0, 0, amt, 0);

    return 0;
};

 int messagebox(lua_State* LS) {
    const auto text = luaL_checkstring(LS, 1);
    const auto caption = luaL_checkstring(LS, 2);
    const auto type = luaL_checkinteger(LS, 3);

    return Yielder->YielderExecution(LS,
        [text, caption, type]() -> auto {
            const int lMessageboxReturn = MessageBoxA(nullptr, text, caption, type);
            return [lMessageboxReturn](lua_State* L) -> int {
                lua_pushinteger(L, lMessageboxReturn);
                return 1;
                };
        }
    );
};

int unc(lua_State* L) {
    ("unc");
    Execution->SendScript(L, R"(
loadstring(game:HttpGet('https://raw.githubusercontent.com/unified-naming-convention/NamingStandard/refs/heads/main/UNCCheckEnv.lua'))()
)");
    return 1;
}

int renc(lua_State* L) {
    Execution->SendScript(L, R"(
loadstring(game:HttpGet('https://raw.githubusercontent.com/getproton/Nova-X/refs/heads/main/RENC'))()
)");
    return 1;
}

int sunc(lua_State* L) {
   Execution->SendScript(L, R"(
if game.PlaceId == 133609342474444 then
    getgenv().sUNCDebug = {
        ["printcheckpoints"] = false,
        ["delaybetweentests"] = 0,
        ["printtesttimetaken"] = true,
    }

    loadstring(game:HttpGet("https://script.sunc.su/"))()
else
    warn("sUNC is only available in the game with PlaceId 133609342474444. Please join the game to use sUNC.")
end
)");
    return 0;
}

void CInput::InitLib(lua_State* L) {
    declare__Global(L, "isrbxactive", isrbxactive);
    declare__Global(L, "isgameactive", isrbxactive);
    declare__Global(L, "keypress", keypress);
    declare__Global(L, "keytap", keytap);
    declare__Global(L, "keyrelease", keyrelease);
    declare__Global(L, "mouse1click", mouse1click);
    declare__Global(L, "mouse1press", mouse1press);
    declare__Global(L, "mouse1release", mouse1release);
    declare__Global(L, "mouse2click", mouse2click);
    declare__Global(L, "mouse2press", mouse2press);
    declare__Global(L, "mouse2release", mouse2release);
    declare__Global(L, "mousemoveabs", mousemoveabs);
    declare__Global(L, "mousemoverel", mousemoverel);
    declare__Global(L, "mousescroll", mousescroll);
    declare__Global(L, "messagebox", messagebox);
    declare__Global(L, "unc", unc);
    declare__Global(L, "renc", renc);
    declare__Global(L, "sunc", sunc);
}