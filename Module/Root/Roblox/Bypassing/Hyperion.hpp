#pragma once
#include "Includes.hpp"

struct Bypasser
{
    void Bitmap(HMODULE module)
    {
        MODULEINFO mi;
        GetModuleInformation(GetCurrentProcess(), module, &mi, sizeof(mi));

        auto base = (uintptr_t)module;
        auto end = base + mi.SizeOfImage;
        auto bmp = *(uintptr_t*)Offsets::Bitmap;

        for (auto p = base; p < end; p += 0x1000)
            *(uint8_t*)(bmp + (p >> 0x13)) |= 1 << ((p >> 0x10) & 7);
    }
};

inline auto Patching = std::make_unique<Bypasser>();
