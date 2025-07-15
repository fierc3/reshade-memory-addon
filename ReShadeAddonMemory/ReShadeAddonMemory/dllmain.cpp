#include "pch.h"
#include <reshade.hpp>
#include <windows.h>

static HANDLE hMapFile = nullptr;
static float* sharedAlpha = nullptr;

static void on_present(reshade::api::effect_runtime* runtime)
{
    if (!sharedAlpha) return;

    float alpha = *sharedAlpha;

    reshade::api::effect_uniform_variable uniform = runtime->find_uniform_variable(nullptr, "mem_Alpha");
    if (uniform.handle != 0)
    {
        runtime->set_uniform_value_float(uniform, &alpha, 1);
    }
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID)
{
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        hMapFile = OpenFileMappingA(FILE_MAP_READ, FALSE, "Local\\ReShadeAddonShared");
        if (hMapFile)
            sharedAlpha = (float*)MapViewOfFile(hMapFile, FILE_MAP_READ, 0, 0, sizeof(float));

        if (!reshade::register_addon(hinstDLL))
            return FALSE;
        reshade::register_event<reshade::addon_event::reshade_present>(&on_present);
        break;

    case DLL_PROCESS_DETACH:
        reshade::unregister_event<reshade::addon_event::reshade_present>(&on_present);
        reshade::unregister_addon(hinstDLL);

        if (sharedAlpha) UnmapViewOfFile(sharedAlpha);
        if (hMapFile) CloseHandle(hMapFile);
        break;
    }
    return TRUE;
}
