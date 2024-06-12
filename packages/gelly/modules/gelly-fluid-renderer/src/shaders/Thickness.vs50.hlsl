#include "ThicknessStructs.hlsli"

Buffer<float4> g_Absorption : register(t0);

struct VS_REAL_INPUT {
    float4 Pos : SV_Position;
    uint ID : SV_VertexID;
};

VS_INPUT main(VS_REAL_INPUT input) {
    VS_INPUT output = (VS_INPUT)0;
	input.Pos.xyz *= g_ScaleDivisor;
    // This is pretty much just a pass-through shader (hence returning an input), but we do want to make sure that the w-component of the position is 1.0
    output.Pos = float4(input.Pos.xyz, 1.0f);
    output.Absorption = g_Absorption.Load(input.ID).xyz;
    return output;
}