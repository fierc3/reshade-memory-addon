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


float4 PS_Main(float4 pos : SV_Position, float2 texcoord : TEXCOORD) : SV_Target
{
    // Center of screen in UV space
    float2 center = float2(mem_positionX, mem_positionY);

    // Distance from current pixel to center
    float dist = distance(texcoord, center);

    // Sample color from backbuffer
    float4 color = tex2D(ReShade::BackBuffer, texcoord);

    // Compute smooth edge fade
    float fade = smoothstep(Radius, Radius + EdgeSoftness, dist);

    // Interpolate between original color and black
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
