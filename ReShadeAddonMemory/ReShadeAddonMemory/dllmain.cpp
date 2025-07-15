#include "pch.h"
#include <reshade.hpp>
#include <Windows.h> // For GetCursorPos
#include <algorithm> // For std::clamp

static float uAlphaValue = 0.0f;

static void on_present(reshade::api::effect_runtime* runtime)
{
    // Get screen dimensions
    POINT cursor;
    if (GetCursorPos(&cursor))
    {
        int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        uAlphaValue = static_cast<float>(cursor.x) / static_cast<float>(screenWidth);
        uAlphaValue = std::clamp(uAlphaValue, 0.0f, 1.0f);
    }

    // Find and set the shader uniform
    reshade::api::effect_uniform_variable uniform = runtime->find_uniform_variable(nullptr, "mem_Alpha");
    if (uniform.handle != 0)
    {
        runtime->set_uniform_value_float(uniform, &uAlphaValue, 1);
    }
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID)
{
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        if (!reshade::register_addon(hinstDLL))
            return FALSE;
        reshade::register_event<reshade::addon_event::reshade_present>(&on_present);
        break;

    case DLL_PROCESS_DETACH:
        reshade::unregister_event<reshade::addon_event::reshade_present>(&on_present);
        reshade::unregister_addon(hinstDLL);
        break;
    }
    return TRUE;
}
