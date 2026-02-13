#include "FOVPatch.h"

bool FOVPatch::Install()
{
    logger::info("Installing FirstPerson FOV patch...");

    auto hookAddr = REL::RelocationID(49852, 50784).address() + 0x1A6;

    SKSE::AllocTrampoline(14);
    auto& trampoline = SKSE::GetTrampoline();

    UpdateCamera::func = trampoline.write_call<5>(hookAddr, UpdateCamera::thunk);

    logger::info("FirstPerson FOV patch installed at 0x{:X}.", hookAddr);
    return true;
}

