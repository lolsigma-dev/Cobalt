#pragma once
#include "Scheduling.hpp"
#include "Includes.hpp"
std::vector<uintptr_t> Jobs;
uintptr_t Address = 0;

void CScheduler::SetProtoCaps(Proto* Proto, uintptr_t* Capabilities)
{
    Proto->userdata = Capabilities;
    for (int i = 0; i < Proto->sizep; ++i)
    {
        SetProtoCaps(Proto->p[i], Capabilities);
    }
}

void CScheduler::SetThreadCaps(lua_State* L, int Identity, uintptr_t Capabilities)
{
    L->userdata->Identity = Identity;
    L->userdata->Capabilities = Capabilities;
}


uintptr_t CScheduler::DataModel()
{
    uintptr_t FakeDM = *reinterpret_cast<uintptr_t*>(Offsets::External::TaskScheduler::FakeDMPointer);
    uintptr_t DM = *reinterpret_cast<uintptr_t*>(FakeDM + Offsets::External::TaskScheduler::FakeDMtoDM);
    return DM;
}

uintptr_t CScheduler::ScriptContext(uintptr_t DataModel)
{
    uintptr_t Children = *reinterpret_cast<uintptr_t*>(DataModel + Offsets::External::DataModel::Children);
    uintptr_t SC = *reinterpret_cast<uintptr_t*>(*reinterpret_cast<uintptr_t*>(Children) + Offsets::External::TaskScheduler::ScriptContext);
    return SC;
}

lua_State* CScheduler::LuaState(uintptr_t ScriptContext)
{
    uintptr_t dataModel = Scheduler->DataModel();
    uintptr_t StateFn = REBASE(0xD7BA30);
    using decrypt_state_t = int64_t(*)(int64_t, uint64_t*, uint64_t*);
    auto GetState = reinterpret_cast<decrypt_state_t>(StateFn);
    uint64_t a2 = 0;
    uint64_t a3 = 0;
    int64_t scriptContext = static_cast<int64_t>(ScriptContext);
    int64_t state = GetState(scriptContext, &a2, &a3);
    return reinterpret_cast<lua_State*>(state);
}

bool CScheduler::IsValidPointer(void* ptr, size_t size) {
    if (!ptr) return false;

    MEMORY_BASIC_INFORMATION mbi;
    if (VirtualQuery(ptr, &mbi, sizeof(mbi)) == 0)
        return false;

    uintptr_t start = reinterpret_cast<uintptr_t>(ptr);
    uintptr_t end = start + size;
    uintptr_t regionStart = reinterpret_cast<uintptr_t>(mbi.BaseAddress);
    uintptr_t regionEnd = regionStart + mbi.RegionSize;

    return (start >= regionStart && end <= regionEnd) &&
        (mbi.State == MEM_COMMIT) &&
        ((mbi.Protect & (PAGE_READONLY | PAGE_READWRITE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE)) != 0);
}

__int64 CScheduler::GetPlaceID() {
    __int64 DataModel = CScheduler::DataModel();
    if (!DataModel)
        return 0;

    auto PlaceIdPtr = reinterpret_cast<__int64*>(DataModel + Offsets::External::TaskScheduler::PlaceId);
    if (!PlaceIdPtr)
        return 0;

    return *PlaceIdPtr;
}

bool CScheduler::GameIsLoaded(uintptr_t DataModel) {
    uintptr_t GM = *(uintptr_t*)(DataModel + Offsets::External::DataModel::GameLoaded);
    return GM == 31;
}

void CScheduler::UpdateJobs() {
    Jobs.clear();

    uintptr_t JobsStart = *(uintptr_t*)(Address + Offsets::External::TaskScheduler::JobStart);
    uintptr_t JobsEnd = *(uintptr_t*)(Address + Offsets::External::TaskScheduler::JobStart + sizeof(void*));

    for (auto i = JobsStart; i < JobsEnd; i += 0x10) {
        uintptr_t Job = *(uintptr_t*)i;
        if (!Job) continue;

        std::string* JobName = reinterpret_cast<std::string*>(Job + Offsets::External::TaskScheduler::JobEnd);
        if (JobName && JobName->length() > 0) Jobs.push_back(Job);
    }
}

uintptr_t CScheduler::GetJobByName(const std::string& Name) {
    for (auto Job : Jobs) {
        if (*(std::string*)(Job + Offsets::External::TaskScheduler::JobName) == Name)
            return Job;
    }

    return 0;
}
