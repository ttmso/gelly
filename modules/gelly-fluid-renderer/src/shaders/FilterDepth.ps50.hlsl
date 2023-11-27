#include "FluidRenderCBuffer.hlsli"
#include "ScreenQuadStructs.hlsli"

Texture2D InputDepth : register(t0);
SamplerState InputDepthSampler : register(s0);

struct PS_OUTPUT {
    float4 Color : SV_Target0;
};

static const float g_blurWeights[9] = {
    0.0625f, 0.125f, 0.0625f,
    0.125f, 0.25f, 0.125f,
    0.0625f, 0.125f, 0.0625f
};

float4 BlurDepth(float2 tex) {
    float4 color = 0.0f;
    float2 texelSize = 1.0f / float2(g_ViewportWidth, g_ViewportHeight);
    
    color += InputDepth.Sample(InputDepthSampler, tex + float2(-texelSize.x, -texelSize.y)) * g_blurWeights[0];
    color += InputDepth.Sample(InputDepthSampler, tex + float2(0.0f, -texelSize.y)) * g_blurWeights[1];
    color += InputDepth.Sample(InputDepthSampler, tex + float2(texelSize.x, -texelSize.y)) * g_blurWeights[2];
    color += InputDepth.Sample(InputDepthSampler, tex + float2(-texelSize.x, 0.0f)) * g_blurWeights[3];
    color += InputDepth.Sample(InputDepthSampler, tex) * g_blurWeights[4];
    color += InputDepth.Sample(InputDepthSampler, tex + float2(texelSize.x, 0.0f)) * g_blurWeights[5];
    color += InputDepth.Sample(InputDepthSampler, tex + float2(-texelSize.x, texelSize.y)) * g_blurWeights[6];
    color += InputDepth.Sample(InputDepthSampler, tex + float2(0.0f, texelSize.y)) * g_blurWeights[7];
    color += InputDepth.Sample(InputDepthSampler, tex + float2(texelSize.x, texelSize.y)) * g_blurWeights[8];

    return color;
}

PS_OUTPUT main(VS_OUTPUT input) {
    PS_OUTPUT output = (PS_OUTPUT)0;
    output.Color = BlurDepth(input.Tex);

    float depth = output.Color.x;
    float4 pos = float4(input.Pos.xyz, 1.0f);
    pos.xy /= float2(g_ViewportWidth, g_ViewportHeight);
    pos.xy = float2(pos.x * 2.0f - 1.0f, (1.0f - pos.y) * 2.0f - 1.0f);
    pos.z = depth;
    pos = mul(g_InverseProjection, pos);
    pos = mul(g_InverseView, pos);
    pos.xyz /= pos.w;
    
    float3 normal = normalize(cross(ddy_fine(pos.xyz), ddx_fine(pos.xyz)));
    output.Color = float4(normal, 1.0f);
    return output;
}