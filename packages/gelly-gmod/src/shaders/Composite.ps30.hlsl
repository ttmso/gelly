#include "NDCQuadStages.hlsli"
#include "util/ReconstructHighBits.hlsli"
#include "material/FluidMaterial.hlsli"
#include "material/Absorption.hlsli"
#include "material/Schlicks.hlsli"
#include "source-engine/AmbientCube.hlsli"
#include "material/Diffuse.hlsli"
#include "util/CMRMap.hlsli"
#include "util/TemperatureMap.hlsli"

// useful defines for offline debugging
//#define NORMALS_VIEW
//#define NORMALS_VIEW_DEBUG_CURVATURE
//#define ABSORPTION_AS_HEATMAP_VIEW
//#define NORMAL_ALPHA_AS_HEATMAP_VIEW

sampler2D depthTex : register(s0);
sampler2D normalTex : register(s1);
sampler2D positionTex : register(s2);
sampler2D backbufferTex : register(s3);
sampler2D thicknessTex : register(s4);
samplerCUBE cubemapTex : register(s5);
sampler2D absorptionTex : register(s6);

float4 eyePos : register(c0);
float4 refractAndCubemapStrength : register(c1);

struct CompositeLight {
    float4 LightInfo;
    float4 Position;
    float4 Enabled;
};

CompositeLight lights[2] : register(c2); // next reg that can be used is c8 (2 + 6)
float4 aspectRatio : register(c8);
float4 ambientCube[6] : register(c9);
FluidMaterial material : register(c15);
float4x4 viewProjMatrix : register(c17);

struct PS_OUTPUT {
    float4 Color : SV_TARGET0;
    float Depth : SV_DEPTH;
};

float3 ComputeSpecularRadianceFromLights(float3 position, float3 normal, float3 eyePos) {
    float3 radiance = float3(0.0, 0.0, 0.0);

    [unroll]
    for (int i = 0; i < 2; i++) {
        float3 lightDir = normalize(lights[i].Position.xyz - position);
        float3 reflectionDir = reflect(-lightDir, normal);
        float3 eyeDir = normalize(eyePos - position);
        float specularRadiance = pow(max(dot(reflectionDir, eyeDir), 0.0), 64.0) * 4.f; // Source-engine-like specular

        radiance += lights[i].LightInfo.xyz * specularRadiance * lights[i].Enabled.x;
    }

    return max(radiance, float3(0.0, 0.0, 0.0));
}

float2 ApplyRefractionToUV(in float2 tex, in float thickness, in float3 normal, in float3 pos, in float3 eyeDir) {
	float3 refractionDir = refract(eyeDir, normal, 1.f / material.r_st_ior.z);
	float3 refractPos = pos + refractionDir * 8.f;
	float4 refractPosClip = mul(viewProjMatrix, float4(refractPos, 1.f));
	float2 refractUV = refractPosClip.xy / refractPosClip.w;
	refractUV = 0.5f * refractUV + 0.5f;
	refractUV.y = 1.f - refractUV.y;

    return refractUV;
}

float3 SampleTransmission(in float2 tex, in float thickness, in float3 pos, in float3 eyeDir, in float3 normal, in float3 absorption) {
    float2 uv = ApplyRefractionToUV(tex, thickness, normal, pos, eyeDir);
    float3 transmission = tex2D(backbufferTex, uv).xyz;
    transmission *= absorption;
    return transmission;
}

#define UNDERWATER_DEPTH_MINIMUM 0.7f
float4 Shade(VS_INPUT input, float projectedDepth) {
    #ifdef ABSORPTION_AS_HEATMAP_VIEW
        return float4(util::TemperatureMapFloat(tex2D(absorptionTex, input.Tex).x), 1.f);
    #endif

    #ifdef NORMAL_ALPHA_AS_HEATMAP_VIEW
        return float4(util::TemperatureMapFloat(tex2D(normalTex, input.Tex).w), 1.f);
    #endif
    
    if (projectedDepth <= UNDERWATER_DEPTH_MINIMUM) {
        float thickness = tex2D(thicknessTex, input.Tex).x;
        float3 absorption = ComputeAbsorption(NormalizeAbsorption(tex2D(absorptionTex, input.Tex).xyz, thickness), thickness);
        float3 transmission = tex2D(backbufferTex, input.Tex).xyz;

        return float4(transmission * absorption, 1.f); // simple underwater effect
    }

    float thickness = tex2D(thicknessTex, input.Tex).x;
    float3 absorption = ComputeAbsorption(NormalizeAbsorption(tex2D(absorptionTex, input.Tex).xyz, thickness), thickness);
    float3 position = tex2D(positionTex, input.Tex).xyz;
    float3 normal = tex2D(normalTex, input.Tex).xyz;
    
    float3 eyeDir = normalize(eyePos.xyz - position);
    float3 reflectionDir = reflect(-eyeDir, normal);

    float fresnel = Schlicks(max(dot(normal, eyeDir), 0.0), material.r_st_ior.z);

    float3 specular = texCUBE(cubemapTex, reflectionDir).xyz * refractAndCubemapStrength.y + ComputeSpecularRadianceFromLights(position, normal, eyePos.xyz);
    float3 diffuseIrradiance = SampleAmbientCube(ambientCube, normal);
    float3 diffuse = Fr_DisneyDiffuse(
        GetNdotV(normal, eyeDir),
        GetNdotL(normal, normal), // ambient light is coming from everywhere
        GetLdotH(normal, GetHalfwayDir(eyeDir, normal)),
        material.r_st_ior.x * material.r_st_ior.x
    ) * diffuseIrradiance * material.diffuseAlbedo;

    float3 specularTransmissionLobe = (1.f - fresnel) * SampleTransmission(input.Tex, thickness, position, eyeDir, normal, absorption) + fresnel * specular;
    // inverse fresnel is already applied to the diffuse lobe
    float3 diffuseSpecularLobe = diffuse + fresnel * specular;
        float3 roughLobe = (1.f - material.r_st_ior.x) * diffuseSpecularLobe + material.r_st_ior.x * diffuse;

    specularTransmissionLobe *= material.r_st_ior.y;
    roughLobe *= (1.f - material.r_st_ior.y);

#if defined(NORMALS_VIEW)
    return float4(normal * 0.5f + 0.5f, 1.f);
#elif defined(NORMALS_VIEW_DEBUG_CURVATURE)
    // curvature is just the ddx and ddy of the normal, but we need to consider the world space distance between pixels
    // so we just divide by the worldspace size of the fragment
    float curvature = length(fwidth(normal));
    return float4(util::CMRMapFloat(curvature), 1.f);
#else
    float3 weight = specularTransmissionLobe + roughLobe;
	return float4(weight, 1.f);
#endif
}

PS_OUTPUT main(VS_INPUT input) {
    float4 depthFragment = tex2D(depthTex, input.Tex);
    if (depthFragment.r >= 1.f) {
        discard;
    }
    
    PS_OUTPUT output = (PS_OUTPUT)0;
    output.Color = Shade(input, depthFragment.r);
    output.Depth = depthFragment.r;
    return output;
}
