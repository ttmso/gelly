#include "FluidRenderCBuffer.hlsli"
#include "SplattingStructs.hlsli"
#include "util/SolveQuadratic.hlsli"

#ifdef NVAPI_ENABLED
#include "util/nv/IntrinsicsSlot.hlsli"
#include "nvHLSLExtns.h"
#include "util/nv/Timing.hlsli"
#endif

#define COMPUTE_HEATMAP_TO_ALBEDO

#if defined(COMPUTE_HEATMAP_TO_ALBEDO) && !defined(NVAPI_ENABLED)
#error "NVAPI must be enabled to compute heatmap to albedo"
#endif

float sqr(float x) {
    return x * x;
}

PS_OUTPUT main(GS_OUTPUT input) {
    PS_OUTPUT output = (PS_OUTPUT)0;

	float4x4 invQuadric = input.InvQuadric;
    float4 position = input.Pos;

    float2 invViewport = rcp(float2(
    	g_ViewportWidth,
        g_ViewportHeight
    ));
	
    float4 ndcPos = float4(
        mad(input.Pos.x, invViewport.x * 2.f, -1.f),
        (1.f - input.Pos.y * invViewport.y) * 2.f - 1.f,
        0.f,
        1.f
    );

START_TIMER;
    float4 viewDir = mul(g_InverseProjection, ndcPos);
    float4 dir = mul(invQuadric, float4(viewDir.xyz, 0.f));
    float4 origin = invQuadric._m03_m13_m23_m33;
END_TIMER;

    // Solve the quadratic equation
    float a = dot(dir.xyz, dir.xyz);
    float b = -mad(dir.w, origin.w, -dot(dir.xyz, origin.xyz));
    float c = dot(origin.xyz, origin.xyz) - sqr(origin.w);

    float minT, maxT;
	[branch]
    if (!SolveQuadratic(a, 2.f * b, c, minT, maxT)) {
        discard;
    }

	minT = max(minT, 0.f);
	clip(maxT);

    float3 eyePos = minT * viewDir.xyz;
    float4 rayNDCPos = mul(g_Projection, float4(eyePos, 1.f));

    float projectionDepth = rayNDCPos.z * rcp(rayNDCPos.w);
    float eyeDepth = eyePos.z;

	#ifdef COMPUTE_HEATMAP_TO_ALBEDO
	output.Absorption = float4(dt, 0, 0, 1);
	#else
	output.Absorption = float4(input.Absorption.xyz, 1.f);
	#endif

	output.FrontDepth = float2(projectionDepth, -eyeDepth);

    float internalDistance = abs(maxT - minT);
    float featheringGaussianWidth = 0.73f; // arbitrary value
    float thicknessFeatheringGaussian = exp(-sqr(internalDistance) / 2.f * sqr(featheringGaussianWidth));

    // we want lower thickness for thin parts, so we will invert the value
    thicknessFeatheringGaussian = 1.f - thicknessFeatheringGaussian;
	output.Thickness = 0.1f * thicknessFeatheringGaussian; // arbitrary value, gets added up to form the thickness
    return output;
}
