cbuffer fluidRender : register(b0) {
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

	float g_ScaleDivisor;
	float3 padding3;
};