// Contact @binninwl_ for any questions
#include <Windows.h>
#include <thread>
#include "Includes.hpp"
#include <Bypassing/Hyperion.hpp>
#include "Common/VEH/VEH.hpp"
#include <Communications/Communications.hpp>

void Thread()
{
	CVEH::Start();
	Logger::Init();

	MessageBoxA(nullptr, "Injected", "Cobalt | EntryPoint", MB_ICONINFORMATION);

	TPService->Initialize();

	ROBLOX::Print(2, "Exploit successfully migrated into Roblox");

	std::thread([] {
		AutoExecution->Run();
		}).detach();
}


bool __stdcall DllMain(HMODULE Module, uintptr_t Reason, void*) {
	if (Reason == DLL_PROCESS_ATTACH)
		std::thread(Thread).detach();
	return true;
}
