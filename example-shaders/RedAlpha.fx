#include "ReShade.fxh"


uniform float uAlpha <
    source = "ReShadeAddonMemory";
> = 0.5;


texture2D texColor : COLOR;
sampler SamplerColor
{
    Texture = texColor;
};

float4 MainPass(float4 pos : SV_Position, float2 uv : TEXCOORD) : SV_Target
{
    float4 original = tex2D(SamplerColor, uv);
    float4 red = float4(1.0, 0.0, 0.0, 1.0);
    return lerp(original, red, uAlpha); // blend red based on alpha
}

technique EyeBlur
{
    pass
    {
        VertexShader = PostProcessVS;
        PixelShader  = MainPass;
    }
}
