#include "ReShade.fxh"

uniform float Radius = 0.2;
uniform float EdgeSoftness = 0.05;

uniform float4 BackgroundColor <
    ui_label = "Background Color";
    ui_type = "color";
> = float4(0.1, 0.1, 0.1, 1.0); // Default: dark gray

uniform float mem_positionX <
    source = "ReShadeAddonMemory";
> = 0.5;

uniform float mem_positionY <
    source = "ReShadeAddonMemory";
> = 0.5;

// Parameterizable offsets
uniform float XOffset <
    ui_label = "X Offset";
    ui_tooltip = "Horizontal offset between the two circles";
> = 0.25;

uniform float YOffset <
    ui_label = "Y Offset";
    ui_tooltip = "Vertical adjustment for circle centers";
> = 0.1;

float4 PS_Main(float4 pos : SV_Position, float2 texcoord : TEXCOORD) : SV_Target
{
    // Centers of the two circles with configurable offsets
    float2 centerLeft  = float2(mem_positionX - XOffset, mem_positionY + YOffset);
    float2 centerRight = float2(mem_positionX + XOffset, mem_positionY + YOffset);

    // Distances from current pixel to both centers
    float distLeft  = distance(texcoord, centerLeft);
    float distRight = distance(texcoord, centerRight);

    // Sample color from backbuffer
    float4 color = tex2D(ReShade::BackBuffer, texcoord);

    // Compute smooth edge fade for both circles
    float fadeLeft  = smoothstep(Radius, Radius + EdgeSoftness, distLeft);
    float fadeRight = smoothstep(Radius, Radius + EdgeSoftness, distRight);

    // Use the minimum fade → pixel is inside either circle
    float fade = min(fadeLeft, fadeRight);

    // Interpolate between original color and background
    return lerp(color, BackgroundColor, fade);
}

technique Glaucoma
{
    pass
    {
        VertexShader = PostProcessVS;
        PixelShader = PS_Main;
    }
}