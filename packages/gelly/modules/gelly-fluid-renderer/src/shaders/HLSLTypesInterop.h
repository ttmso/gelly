#ifndef GELLY_HLSL_TYPES_INTEROP_H
#define GELLY_HLSL_TYPES_INTEROP_H

namespace gelly::renderer::cbuffer {
	struct float4 {
		float x;
		float y;
		float z;
		float w;
	};

	struct float3 {
		float x;
		float y;
		float z;
	};

	struct float2 {
		float x;
		float y;
	};

	struct float4x4 {
		float m[4][4];
	};
}

#endif // GELLY_HLSL_TYPES_INTEROP_H