#pragma once
#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1
#include <Windows.h>
#include <string>
#include <TlHelp32.h>
#include <thread>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <map>      
#include <cstdlib>   
#include <stdexcept> 
#include <ctime>    
#include <cctype>    
#include <queue>
#include <random>    
#include <Psapi.h>
#include <sstream>

#include <lua.h>
#include <lualib.h>
#include <lstate.h>
#include "VM/src/lcommon.h"
#include "VM/src/lstring.h"
#include "VM/src/lfunc.h"
#include "VM/src/lmem.h"
#include "VM/src/lgc.h"
#include "VM/src/ltable.h"
#include "VM/src/lobject.h"
#include "VM/src/lapi.h"
#include "Reflections.hpp"
#include "VM/src/ldo.h"

#include <Bytecode/Bytecode.hpp>
#include <Engine.hpp>

#include <cryptopp/md5.h>
#include <cryptopp/sha3.h>
#include <cryptopp/sha.h>
#include <cryptopp/base64.h>
#include <cryptopp/hex.h>
#include <cryptopp/rdrand.h>
#include <cryptopp/modes.h>
#include <cryptopp/sha256_armv4.h>
#include <cryptopp/aes.h>

#include <nlohmann/json.hpp>

#include "zstd/include/zstd/zstd.h"
#include "Compiler/include/Luau/Compiler.h"
#include "Compiler/include/luacode.h"
#include "Common/include/Luau/BytecodeUtils.h"
#include "Compiler/include/Luau/BytecodeBuilder.h"

#include <Execution/Execution.hpp>
#include <Scheduler/Scheduling.hpp>
#include <TPService/Handler.hpp>
#include <Overlay/Overlay.hpp>
#include <Rendering/imgui-notify.h>
#include <Common/Logger/Logger.hpp>
#include <Yieldworker/Yielder.hpp>
#include <AutoExecute/Autoexec.hpp>