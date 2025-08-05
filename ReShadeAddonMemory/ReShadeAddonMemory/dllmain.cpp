#include "pch.h"
#include <reshade.hpp>
#include <windows.h>

static const size_t MaxNameLength = 64;
static HANDLE hMapFile = nullptr;
static uint8_t* sharedMemory = nullptr;

/*
Memory Structure

Offset 0:   Name "mem_positionX"  (64 bytes)
Offset 64:  float value X         (4 bytes)

Offset 68:  Name "mem_positionY"  (64 bytes)
Offset 132: float value Y         (4 bytes)

*/

static void on_present(reshade::api::effect_runtime* runtime)
{
    if (!sharedMemory)
        return;

    const size_t EntrySize = MaxNameLength + sizeof(float);

    for (int i = 0; i < 2; ++i) // Read both entries: X and Y
    {
        uint8_t* entryPtr = sharedMemory + i * EntrySize;

        // Read name
        char name[MaxNameLength]{};
        memcpy(name, entryPtr, MaxNameLength);
        name[MaxNameLength - 1] = '\0'; // Ensure null-terminated

        // Read float value
        float value = *reinterpret_cast<float*>(entryPtr + MaxNameLength);

        auto uniform = runtime->find_uniform_variable(nullptr, name);
        if (uniform.handle != 0)
        {
            runtime->set_uniform_value_float(uniform, &value, 1);
        }
    }
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID)
{
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        hMapFile = OpenFileMappingA(FILE_MAP_READ, FALSE, "Local\\ReShadeAddonShared");
        if (hMapFile)
        {
            sharedMemory = static_cast<uint8_t*>(MapViewOfFile(hMapFile, FILE_MAP_READ, 0, 0, MaxNameLength + sizeof(float)));
        }

        if (!reshade::register_addon(hinstDLL))
            return FALSE;
        reshade::register_event<reshade::addon_event::reshade_present>(&on_present);
        break;

    case DLL_PROCESS_DETACH:
        reshade::unregister_event<reshade::addon_event::reshade_present>(&on_present);
        reshade::unregister_addon(hinstDLL);

        if (sharedMemory) UnmapViewOfFile(sharedMemory);
        if (hMapFile) CloseHandle(hMapFile);
        break;
    }
    return TRUE;
}
