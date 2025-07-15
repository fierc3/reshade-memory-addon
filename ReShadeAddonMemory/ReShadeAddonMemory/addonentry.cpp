#include <reshade.hpp>
#include <random>

static float random_alpha = 0.0f;

void on_frame(reshade::api::command_list* cmd_list, reshade::api::effect_runtime* runtime)
{
    random_alpha = static_cast<float>(rand()) / RAND_MAX;

    auto uniform = runtime->find_uniform_variable(nullptr, "uAlpha");
    if (uniform.handle != 0)
    {
        runtime->set_uniform_value_float(uniform, &random_alpha, 1);
    }
}

extern "C" __declspec(dllexport) const char* NAME = "RandomAlphaAddon";
extern "C" __declspec(dllexport) const char* DESCRIPTION = "Sets a random alpha value every frame.";

extern "C" __declspec(dllexport) void AddonInit()
{
    reshade::register_event<reshade::addon_event::reshade_begin_effects>(on_frame);
}

extern "C" __declspec(dllexport) void AddonUninit()
{
    reshade::unregister_event<reshade::addon_event::reshade_begin_effects>(on_frame);
}
