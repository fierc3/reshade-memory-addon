#pragma comment(lib, "VarjoLib.lib")

#include <windows.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <cmath>
#include <cstring>

#include <Varjo.h>
#include <Varjo_types.h>

// ===== Shared memory schema =====
static const int   MaxNameLength = 64;
static const int   EntrySize = MaxNameLength + sizeof(float);
static const char* MapName = "Local\\ReShadeAddonShared";

static const char* UniformX = "mem_positionX";
static const char* UniformY = "mem_positionY";

constexpr double PI = 3.14159265358979323846;

inline void WriteEntry(uint8_t* base, int index, const char* name, float value) {
    uint8_t* slot = base + index * EntrySize;
    size_t len = strlen(name);
    if (len >= MaxNameLength) len = MaxNameLength - 1;
    std::memset(slot, 0, MaxNameLength);
    std::memcpy(slot, name, len);
    *reinterpret_cast<float*>(slot + MaxNameLength) = value;
}

int main() {
    std::cout << R"(
============================================================
    Varjo Aero Eye Tracking -> ReShadeAddonMemory  (Example)
============================================================
This example shows how to use Varjo Aero eye tracking with
ReShadeAddonMemory.

ReShadeAddonMemory: A minimal ReShade add-on that allows real-time
control of shader uniforms (currently float values) from an external
program using shared memory.

Writes uniforms:  mem_positionX, mem_positionY   (range 0..1)
Shared memory:    Local\ReShadeAddonShared

Starting eye tracking stream...
    )";

    // Open/create shared memory and map it
    HANDLE hMap = CreateFileMappingA(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE,
        0, EntrySize * 2, MapName);
    if (!hMap) {
        std::cerr << "CreateFileMappingA failed: " << GetLastError() << "\n";
        return 1;
    }
    uint8_t* mem = reinterpret_cast<uint8_t*>(
        MapViewOfFile(hMap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0));
    if (!mem) {
        std::cerr << "MapViewOfFile failed: " << GetLastError() << "\n";
        CloseHandle(hMap);
        return 1;
    }

    // Initialize names so the addon can discover them immediately
    WriteEntry(mem, 0, UniformX, 0.5f);
    WriteEntry(mem, 1, UniformY, 0.5f);

    // Varjo session + gaze init
    varjo_Session* session = varjo_SessionInit();
    if (!session) {
        std::cerr << "Failed to initialize Varjo session (is Varjo Base running?).\n";
        UnmapViewOfFile(mem);
        CloseHandle(hMap);
        return 1;
    }
    varjo_GazeInit(session);

    std::cout << "Streaming gaze to shared memory as mem_positionX/mem_positionY...\n";

    // Angle-based mapping settings (tune to taste/fit your HMD view)
    const double hFovDeg = 100.0;  // horizontal FOV coverage you want on screen
    const double vFovDeg = 90.0;   // vertical FOV coverage
    const double hHalf = (hFovDeg * 0.5) * PI / 180.0;
    const double vHalf = (vFovDeg * 0.5) * PI / 180.0;

    auto clamp01 = [](double v) { return v < 0.0 ? 0.0 : (v > 1.0 ? 1.0 : v); };

    // Keep last valid for nicer behavior on brief invalid frames
    float lastX = 0.5f, lastY = 0.5f;
    const float smoothAlpha = 0.25f; // 0..1, higher = snappier

    // Loop: sample gaze and write percentages each frame (~60 Hz)
    while (true) {
        varjo_Gaze gaze = varjo_GetGaze(session);

        float outX = lastX;
        float outY = lastY;

        if (gaze.status == varjo_GazeStatus_Valid) {
            const varjo_Ray& r = gaze.gaze; // combined gaze ray
            const double fx = r.forward[0];
            const double fy = r.forward[1];
            const double fz = r.forward[2];

            // Log raw forward + origin
            std::cout << "Raw gaze forward = ["
                << fx << ", " << fy << ", " << fz << "] "
                << "origin = ["
                << r.origin[0] << ", "
                << r.origin[1] << ", "
                << r.origin[2] << "]\n";

            // Angle-based mapping: no reliance on Z sign
            // horizontal angle around Y, vertical around X
            const double uAngle = std::atan2(fx, fz);
            const double vAngle = std::atan2(fy, fz);

            // Normalize to [-1,1] across your chosen FOV limits
            double x_ndc = uAngle / hHalf;
            double y_ndc = vAngle / vHalf;

            // Map to [0,1]
            double u = 0.5 * (x_ndc + 1.0);
            double v = 0.5 * (y_ndc + 1.0);

            // If movement feels inverted, flip as needed:
            // u = 1.0 - u; // flip X
            v = 1.0 - v; // flip Y (for Y-down screens)

            // Clamp and smooth
            float targetX = static_cast<float>(clamp01(u));
            float targetY = static_cast<float>(clamp01(v));
            outX = lastX + smoothAlpha * (targetX - lastX);
            outY = lastY + smoothAlpha * (targetY - lastY);

            // Update last valid
            lastX = outX;
            lastY = outY;
        }
        else {
            std::cout << "No valid gaze (status=" << static_cast<int>(gaze.status) << ")\n";
        }

        std::cout << "→ Shader out: (" << outX << ", " << outY << ")\n";

        // 5) Publish to shared memory for ReShade
        WriteEntry(mem, 0, UniformX, outX);
        WriteEntry(mem, 1, UniformY, outY);

        std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60 Hz
    }

    // Cleanup (unreached in this example)
    // varjo_SessionShutDown(session);
    UnmapViewOfFile(mem);
    CloseHandle(hMap);
    return 0;
}
