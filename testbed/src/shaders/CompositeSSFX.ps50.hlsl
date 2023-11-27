#include "NDCQuad.hlsli"

Texture2D GellyDepth : register(t0);
SamplerState GellyDepthSampler {
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Clamp;
    AddressV = Clamp;
};

struct PS_OUTPUT {
    float4 Color : SV_Target0;
    float Depth : SV_Depth;
};

PS_OUTPUT main(VS_INPUT input)
{
    float4 depth = GellyDepth.Sample(GellyDepthSampler, input.Tex);
    if (depth.a == 0.0f) {
        discard;
    }

    PS_OUTPUT output = (PS_OUTPUT)0;
    output.Color = float4(depth.xyz, 1.0f);
    output.Depth = 0.f;

    return output;
}