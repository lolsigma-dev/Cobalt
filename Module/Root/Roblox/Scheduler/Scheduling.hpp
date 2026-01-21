#pragma once
#include <Includes.hpp>
#include <Globals.hpp>
inline uintptr_t PreviousDM;

class CScheduler {
public:
	static void SetProtoCaps(Proto* Proto, uintptr_t* Capabilities);
	static void SetThreadCaps(lua_State* L, int Level, uintptr_t Capabilities);
	static uintptr_t ScriptContext(uintptr_t DataModel);
	static uintptr_t DataModel();
	static lua_State* LuaState(uintptr_t ScriptContext);
	static bool GameIsLoaded(uintptr_t DataModel);
	static bool IsValidPointer(void* ptr, size_t size);
	static __int64 GetPlaceID();	
	static uintptr_t GetJobByName(const std::string& Name);
	static void UpdateJobs();
	
};


class CTaskScheduler {
public:
	static bool Init();
};
inline auto Scheduler = std::make_unique<CScheduler>();
inline auto TaskScheduler = std::make_unique<CTaskScheduler>();