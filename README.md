# ReShadeAddonMemory

A minimal ReShade add-on that enables **real-time control of shader uniforms** from external programs using **shared memory**.  
Currently supports `float` values.

This repo includes:
- **ReShadeAddonMemory** — the C++ addon (core project)
- **CSharpExampleClient** — a simple shared memory writer
- **example-shaders** — a collection of shaders that use the ReShadeAddonMemory
- **VarjoAeroEyeTrackingExample** — an example integration with Varjo Aero eye tracking

---

## 📁 Project Structure

```
ReShadeAddonMemory/
│
├── ReShadeAddonMemory/        ← C++ addon (.dll)
│   └── dllmain.cpp
│
├── CSharpExampleClient/       ← C# shared memory writer
│   └── Program.cs
│
example-shaders/
├── RedAlpha.fx                ← Example shader using mem_Alpha         
│
VarjoAeroEyeTrackingExample/
├── EyeTrackTestCpp.sln        ← Example how to read Varjo Aero eyetracking data and write it to memory
│     
└── README.md
```

---

## 🛠️ How It Works

### Client

The client, which can be written in any programming languages write values in a shared memory:

* A **uniform name** (max 64 bytes, UTF-8, null-terminated, e.g. `"mem_Alpha"`)
* A **float value**
  To shared memory at `Local\ReShadeAddonShared` using `MemoryMappedFile`.

```csharp
accessor.WriteArray(0, nameBytes, 0, nameBytes.Length); // 64 bytes
accessor.Write(64, floatValue);                         // float value
```

---

### ReShade Addon (Reader)

Addon reads shared memory every frame:

1. Extracts the uniform name (e.g. `mem_Alpha`)
2. Extracts the float value
3. Sets the shader uniform if found

```cpp
auto uniform = runtime->find_uniform_variable(nullptr, "mem_Alpha");
runtime->set_uniform_value_float(uniform, &value, 1);
```

---

## Shader

The shader that can use the ReShadeAddonMemory to read the data from the memory and use it for its logic, e.g:

```hlsl
uniform float mem_Alpha < source = "ReShadeAddonMemory"; > = 0.5;

float4 MainPass(float4 pos : SV_Position, float2 uv : TEXCOORD) : SV_Target
{
    float4 original = tex2D(SamplerColor, uv);
    float4 red = float4(1, 0, 0, 1);
    return lerp(original, red, mem_Alpha);
}
```

---

## 🔨 Development Guide

### Requirements

* ReShade 5.x+ with addon support
* A compatible game (e.g., *Oceanhorn*)
* Visual Studio 2022 (C++, .NET SDK)

### Development Build Steps

1. Clone this repo
2. Open Solution in VS 2022+
3. Under Project → Project Properties → C/C++ → General → add the `reshade\include` path (https://github.com/crosire/reshade/tree/main/include).
4. Build `ReShadeAddonMemory` (C++ DLL -> *must match the architecture of target game*)
5. Copy the resulting `.addon` file into your game folder (Reshade must be preinstalled)
6. Place `RedAlpha.fx` in your ReShade shader path (usually `GAME/reshade-shaders/`)
7. Run `CSharpExampleClient` and watch `mem_Alpha` update in real time

## 🎮 Demo: Varjo Aero + Assetto Corsa

### Installation Guide for Varjo Aero & Assetto Corsa Demo

#### 1. Install Varjo Base
- Requires about 3 GB of storage
- Restart your PC after installation

#### 2. Install Assetto Corsa
- Install via Steam
- Important: Launch the game once before proceeding so Steam can install the required redistributables
- For VR: go to game settings → set video output mode to **OpenVR**, then restart the game

#### 3. Install ReShade (with Addon Support)
- Run the ReShade installer and select **acs.exe** (Assetto Corsa executable)
- Choose the pre-selected API (do not manually select Assetto Corsa)
- Download shaders from [example-shaders](https://github.com/fierc3/reshade-memory-addon/tree/main/example-shaders)  
  and place them into the `reshade-shaders` folder created in the Assetto Corsa directory

#### 4. Install ReshadeAddonMemory
- Download the latest `ReshadeAddonMemory.addon` from [Google Drive](https://drive.google.com/drive/folders/1EBNBS96HteNpe_csaqb8hVFpVOEXkDK5?usp=drive_link)  
  or build it yourself in Visual Studio
- Move `ReshadeAddonMemory.addon` into the Assetto Corsa game folder  
  ReShade will auto-detect it
- Make sure you use the 64-bit version (Assetto Corsa runs in 64-bit)

#### 5. Install VarjoEyeTrackerExample
- Download the latest binary from [Google Drive](https://drive.google.com/drive/folders/1EBNBS96HteNpe_csaqb8hVFpVOEXkDK5?usp=drive_link)  
  or build it via Visual Studio
- Run it. If installed correctly, the console window will display gaze values

#### 6. Running the Setup
1. Start **VarjoEyeTrackerExample** first  
2. Then launch Assetto Corsa  
3. In-game, press **HOME** to open the ReShade UI and enable your chosen shader

#### 7. Troubleshooting
- If eye tracking does not work:
  - Check the VarjoEyeTrackerExample console output for gaze values
  - If values are correct there but not in-game, restart both applications (starting the eyetracker first usually works best)
  - Make sure the addon matches the correct architecture (64-bit)



