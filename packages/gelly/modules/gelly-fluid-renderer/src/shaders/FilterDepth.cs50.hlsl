#include "FluidRenderCBuffer.hlsli"
#include "util/EyeToProjDepth.hlsli"

#define FILTER_RADIUS_PIXELS 3 // of course the entire filter is this doubled since it's only a radius
#define FILTER_RADIUS (FILTER_RADIUS_PIXELS * 2)
#define ITERATIONS 1

RWTexture2D<float4> g_unfilteredDepth : register(u0);

[numthreads(4, 1, 1)] // we do 4 pixels at a time
void main(uint3 dtID : SV_DispatchThreadID) {
	// our first step is to initially fetch all of our depth values
	int2 centerPixel = int2(dtID.x, dtID.y);
	if (centerPixel.x >= g_ViewportWidth || centerPixel.y >= g_ViewportHeight) {
		return;
	}

	float2 centerDepth = g_unfilteredDepth.Load(int3(centerPixel, 0)).xy;
	if (centerDepth.r >= 1.f) {
		return;
	}
	
	float depthSum = 0.f;

	[unroll]
	for (int i = -FILTER_RADIUS_PIXELS; i <= FILTER_RADIUS_PIXELS; i++) {
		for (int j = -FILTER_RADIUS_PIXELS; j <= FILTER_RADIUS_PIXELS; j++) {
			int2 pixel = centerPixel + int2(i, j);
			float depth = g_unfilteredDepth.Load(int3(pixel, 0)).g;
			if (depth >= 1.f || isnan(depth)) {
				continue;
			}

			depthSum += depth;
		}
	}

	float depth = depthSum / (FILTER_RADIUS * FILTER_RADIUS);
	g_unfilteredDepth[centerPixel] = float4(centerDepth, 0.f, 1.f);
}