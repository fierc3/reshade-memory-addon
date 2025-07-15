# ReShadeAddonMemory

A minimal ReShade add-on that allows **real-time control of shader uniforms** (currently `float` values) from an external program using **shared memory**.

## 🧠 What is this?

This project demonstrates:

* 🔧 A working **ReShade Addon (`ReShadeAddonMemory`)**
* 🧪 A C# client app (`CSharpExampleClient`) that sends values via `MemoryMappedFile`
* 🎨 A test shader (`RedAlpha.fx`) that reads a float uniform `mem_Alpha` to blend a red tint

Together, they enable external applications to dynamically change shader values — e.g., for simulating eye strain, ambient interaction, and biofeedback, among others.

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
└── README.md
```

---

## 🛠️ How It Works

### 📤 From C# (Writer)

C# app writes:

* A **uniform name** (max 64 bytes, UTF-8, null-terminated, e.g. `"mem_Alpha"`)
* A **float value**
  To shared memory at `Local\ReShadeAddonShared` using `MemoryMappedFile`.

```csharp
accessor.WriteArray(0, nameBytes, 0, nameBytes.Length); // 64 bytes
accessor.Write(64, floatValue);                         // float value
```

---

### 📥 From ReShade Addon (Reader)

Addon reads shared memory every frame:

1. Extracts the uniform name (e.g. `mem_Alpha`)
2. Extracts the float value
3. Sets the shader uniform if found

```cpp
auto uniform = runtime->find_uniform_variable(nullptr, "mem_Alpha");
runtime->set_uniform_value_float(uniform, &value, 1);
```

---

## 🧪 Test Shader: `RedAlpha.fx`

This shader blends the screen red based on the external value of `mem_Alpha`:

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

## 🚀 Getting Started

### ✅ Requirements

* ReShade 5.x+ with addon support
* A compatible game (e.g., *Oceanhorn*)
* Visual Studio 2022 (C++, .NET SDK)

### 📎 Build Steps

1. Clone this repo
2. Open Solution in VS 2022+
3. Under Project → Project Properties → C/C++ → General → add the `reshade\include` path (https://github.com/crosire/reshade/tree/main/include).
4. Build `ReShadeAddonMemory` (C++ DLL -> *must match the architecture of target game*)
5. Copy the resulting `.addon` file into your game folder (Reshade must be preinstalled)
6. Place `RedAlpha.fx` in your ReShade shader path (usually `GAME/reshade-shaders/`)
7. Run `CSharpExampleClient` and watch `mem_Alpha` update in real time


