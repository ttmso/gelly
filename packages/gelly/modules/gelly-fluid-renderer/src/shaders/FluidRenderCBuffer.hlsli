// C++ and HLSL interop macros, we use these to define the constant buffer in
// HLSL and C++ to avoid code duplication.

#ifdef __cplusplus
#include "HLSLTypesInterop.h"
#define CBUFFER_DECLARATION(name) struct name
#else
#define CBUFFER_DECLARATION(name) cbuffer name : register(b0)
#endif

#ifdef __cplusplus
namespace gelly::renderer::cbuffer {
#endif
CBUFFER_DECLARATION(FluidRenderCBufferData) {
	float4x4 g_View;
	float4x4 g_Projection;
	float4x4 g_InverseView;
	float4x4 g_InverseProjection;

	float g_ViewportWidth;
	float g_ViewportHeight;
	float g_ThresholdRatio;
	float g_ParticleRadius;

	float g_NearPlane;
	float g_FarPlane;
	float2 padding;

	float3 g_CameraPosition;
	float g_DiffuseScale;

	float g_DiffuseMotionBlur;
	float3 padding2;
};
#ifdef __cplusplus
}
#endif