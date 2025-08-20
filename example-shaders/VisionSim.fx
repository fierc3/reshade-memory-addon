// VisionSim.fx
#include "ReShade.fxh"

// ===== Shared uniforms (declared once for all techniques) =====
uniform float4 BackgroundColor <
    ui_label = "Mask Color";
    ui_type  = "color";
> = float4(0.0, 0.0, 0.0, 1.0);

uniform float mem_positionX <
    source  = "ReShadeAddonMemory";
    ui_label = "Gaze X";
> = 0.5;

uniform float mem_positionY <
    source  = "ReShadeAddonMemory";
    ui_label = "Gaze Y";
> = 0.5;

// Per-eye placement around shared gaze
uniform float XOffsetLeft  <
    ui_label = "Left X Offset";
    ui_tooltip = "Horizontal offset from gaze for left eye";
> = 0.25;

uniform float XOffsetRight <
    ui_label = "Right X Offset";
    ui_tooltip = "Horizontal offset from gaze for right eye";
> = 0.25;

uniform float YOffsetLeft  <
    ui_label = "Left Y Offset";
    ui_tooltip = "Vertical offset from gaze for left eye";
> = 0.10;

uniform float YOffsetRight <
    ui_label = "Right Y Offset";
    ui_tooltip = "Vertical offset from gaze for right eye";
> = 0.10;

// Per-eye size and softness
uniform float RadiusLeft <
    ui_label = "Left Radius";
    ui_tooltip = "Circle radius for left eye";
> = 0.20;

uniform float RadiusRight <
    ui_label = "Right Radius";
    ui_tooltip = "Circle radius for right eye";
> = 0.20;

uniform float EdgeSoftnessLeft <
    ui_label = "Left Edge Softness";
    ui_tooltip = "Soft edge width for left eye";
> = 0.05;

uniform float EdgeSoftnessRight <
    ui_label = "Right Edge Softness";
    ui_tooltip = "Soft edge width for right eye";
> = 0.05;

// ===== Helpers =====
float2 CenterLeft()
{
    return float2(mem_positionX - XOffsetLeft, mem_positionY + YOffsetLeft);
}

float2 CenterRight()
{
    return float2(mem_positionX + XOffsetRight, mem_positionY + YOffsetRight);
}

// ===== Technique 1: Glaucoma-style tunnel vision (outside fades) =====
float4 PS_Glaucoma(float4 pos : SV_Position, float2 uv : TEXCOORD) : SV_Target
{
    float2 cL = CenterLeft();
    float2 cR = CenterRight();

    float dL = distance(uv, cL);
    float dR = distance(uv, cR);

    // 0 inside, 1 outside, soft transition
    float fadeL = smoothstep(RadiusLeft,  RadiusLeft  + EdgeSoftnessLeft,  dL);
    float fadeR = smoothstep(RadiusRight, RadiusRight + EdgeSoftnessRight, dR);

    // Keep if inside either circle
    float fade = min(fadeL, fadeR);

    float4 src = tex2D(ReShade::BackBuffer, uv);
    return lerp(src, BackgroundColor, fade);
}

technique Glaucoma
{
    pass
    {
        VertexShader = PostProcessVS;
        PixelShader  = PS_Glaucoma;
    }
}

// ===== Technique 2: Scotoma blind spots (inside fades) =====
float4 PS_Scotoma(float4 pos : SV_Position, float2 uv : TEXCOORD) : SV_Target
{
    float2 cL = CenterLeft();
    float2 cR = CenterRight();

    float dL = distance(uv, cL);
    float dR = distance(uv, cR);

    // 1 inside, 0 outside, soft transition
    float occL = 1.0 - smoothstep(RadiusLeft,  RadiusLeft  + EdgeSoftnessLeft,  dL);
    float occR = 1.0 - smoothstep(RadiusRight, RadiusRight + EdgeSoftnessRight, dR);

    // Any eye can occlude
    float occ = max(occL, occR);

    float4 src = tex2D(ReShade::BackBuffer, uv);
    return lerp(src, BackgroundColor, occ);
}

technique Scotoma
{
    pass
    {
        VertexShader = PostProcessVS;
        PixelShader  = PS_Scotoma;
    }
}
