#pragma once

namespace Roblox
{

class EncryptionsHelper
{
public:
    static void Initialize();
    static void Shutdown();
    static bool IsInitialized();
};

}