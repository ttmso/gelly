#ifndef STANDARDPIPELINE_H
#define STANDARDPIPELINE_H

#include <DirectXMath.h>

#include "../Pipeline.h"
#include "StandardTextures.h"
#include "fluidrender/IRenderContext.h"

using namespace DirectX;

struct CompositeConstants {
	float eyePos[3];
	float pad0;
	float absorption[4] = {0.3f, 0.3f, 0.f, 1024.f};
	float refractionStrength = 0.03f;
	float pad1[3];
	XMFLOAT4X4 inverseMVP;
};

static_assert(sizeof(CompositeConstants) % 16 == 0);
class StandardPipeline : public Pipeline {
private:
	struct NDCVertex {
		float x, y, z, w;
		float texX, texY;
	};

	GellyResources gellyResources;
	UnownedResources gmodResources;

	CompositeConstants compositeConstants;

	std::optional<StandardTextures> textures;
	ComPtr<IDirect3DTexture9> backBuffer;
	ComPtr<IDirect3DVertexBuffer9> ndcQuad;
	ComPtr<IDirect3DPixelShader9> compositeShader;
	ComPtr<IDirect3DVertexShader9> quadVertexShader;
	ComPtr<IDirect3DStateBlock9> stateBlock;

	PipelineConfig config;

	void CreateCompositeShader();
	void CreateQuadVertexShader();
	void CreateNDCQuad();
	void CreateStateBlock();
	void CreateBackBuffer();

	void UpdateBackBuffer() const;
	void SetCompositeShaderConstants() const;

	void UpdateGellyRenderParams();
	void RenderGellyFrame();

	void SetCompositeSamplerState(int index, D3DTEXTUREFILTERTYPE filter) const;

public:
	StandardPipeline();
	~StandardPipeline() override = default;

	void CreatePipelineLocalResources(
		const GellyResources &gelly, const UnownedResources &gmod
	) override;

	void SetConfig(const PipelineConfig &config) override;

	void Composite() override;
};

#endif	// STANDARDPIPELINE_H