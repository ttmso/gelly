#ifndef DEPTH_FILTERING_H
#define DEPTH_FILTERING_H

#include <device.h>
#include <helpers/create-gsc-shader.h>

#include <memory>

#include "FilterDepthCS.h"
#include "pipeline-info.h"
#include "pipeline/compute-pipeline.h"

namespace gelly {
namespace renderer {
namespace splatting {
// we take in the input/output depth separately so that we are able
// to create ping pong pipelines for filter iterations
inline auto CreateDepthFilteringPipeline(const PipelineInfo &info)
	-> std::shared_ptr<ComputePipeline> {
	return ComputePipeline::CreateComputePipeline({
		.name = "Filtering depth",
		.device = info.device,
		.computeShader = CS_FROM_GSC(FilterDepthCS, info.device),
		.inputs = {},
		.outputs = {OutputTexture{
			.texture = info.internalTextures->unfilteredEllipsoidDepth,
			.bindFlag = D3D11_BIND_UNORDERED_ACCESS,
			.slot = 0,
			.clearColor = {1.f, 0.f, 1.f, 1.f},
			.clear = false,
		}},
		.constantBuffers = {info.internalBuffers->fluidRenderCBuffer.GetBuffer()
		},
		.repeatCount = 1,
	});
}
}  // namespace splatting
}  // namespace renderer
}  // namespace gelly

#endif	// DEPTH_FILTERING_H
