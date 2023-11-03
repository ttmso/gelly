#include "fluidrender/CD3D11to11SharedTexture.h"

#include <stdexcept>

#include "detail/d3d11/ErrorHandling.h"
#include "fluidrender/IRenderContext.h"

CD3D11to11SharedTexture::CD3D11to11SharedTexture(HANDLE sharedHandle)
	: context(nullptr),
	  texture(nullptr),
	  srv(nullptr),
	  rtv(nullptr),
	  uav(nullptr),
	  sharedHandle(sharedHandle) {}

CD3D11to11SharedTexture::~CD3D11to11SharedTexture() {
	// Unfortunate, but necessary
	// There is UB if a virtual function is called in a destructor
	CD3D11to11SharedTexture::Destroy();
}

void CD3D11to11SharedTexture::AutogenerateDesc() {
	if (texture == nullptr) {
		throw std::logic_error(
			"CD3D11to11SharedTexture::AutogenerateDesc() should not be called "
			"before the texture is created"
		);
	}

	// Pull our texture description from the texture
	D3D11_TEXTURE2D_DESC texDesc = {};
	texture->GetDesc(&texDesc);

	autoGeneratedDesc.width = texDesc.Width;
	autoGeneratedDesc.height = texDesc.Height;

	auto format = (TextureFormat)0;
	switch (texDesc.Format) {
		case DXGI_FORMAT_R8G8B8A8_UNORM:
			format = TextureFormat::R8G8B8A8_UNORM;
			break;
		case DXGI_FORMAT_R32G32B32A32_FLOAT:
			format = TextureFormat::R32G32B32A32_FLOAT;
			break;
		case DXGI_FORMAT_R16G16B16A16_FLOAT:
			format = TextureFormat::R16G16B16A16_FLOAT;
			break;
		default:
			throw std::runtime_error(
				"CD3D11to11SharedTexture::AutogenerateDesc() encountered an "
				"unsupported texture format"
			);
	}

	autoGeneratedDesc.format = format;

	auto access = (TextureAccess)0;
	switch (texDesc.Usage) {
		case D3D11_USAGE_DEFAULT:
			access = TextureAccess::WRITE;
			break;
		case D3D11_USAGE_DYNAMIC:
			access = TextureAccess::READ | TextureAccess::WRITE;
			break;
		default:
			throw std::runtime_error(
				"CD3D11to11SharedTexture::AutogenerateDesc() encountered an "
				"unsupported texture usage"
			);
	}

	autoGeneratedDesc.access = access;
	autoGeneratedDesc.isFullscreen = false;
}

void CD3D11to11SharedTexture::SetDesc(const TextureDesc &desc) {
	// There is no control over the texture description.
	throw std::logic_error(
		"CD3D11to11SharedTexture::SetDesc() should not be called"
	);
}

const TextureDesc &CD3D11to11SharedTexture::GetDesc() const {
	return autoGeneratedDesc;
}

bool CD3D11to11SharedTexture::Create() {
	if (!context) {
		return false;
	}

	if (texture) {
		return false;
	}

	auto *device = static_cast<ID3D11Device *>(
		context->GetRenderAPIResource(RenderAPIResource::D3D11Device)
	);

	ID3D11Resource *resource = nullptr;
	HRESULT hr = device->OpenSharedResource(
		sharedHandle, __uuidof(ID3D11Resource), (void **)&resource
	);

	if (FAILED(hr)) {
		return false;
	}

	hr = resource->QueryInterface(__uuidof(ID3D11Texture2D), (void **)&texture);

	if (FAILED(hr)) {
		return false;
	}

	resource->Release();

	AutogenerateDesc();

	auto format = static_cast<DXGI_FORMAT>(0);
	switch (autoGeneratedDesc.format) {
		case TextureFormat::R8G8B8A8_UNORM:
			format = DXGI_FORMAT_R8G8B8A8_UNORM;
			break;
		case TextureFormat::R32G32B32A32_FLOAT:
			format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			break;
		case TextureFormat::R16G16B16A16_FLOAT:
			format = DXGI_FORMAT_R16G16B16A16_FLOAT;
			break;
		default:
			throw std::runtime_error(
				"CD3D11to11SharedTexture::Create() encountered an "
				"unsupported texture format"
			);
	}

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;

	DX("Failed to create D3D11 RTV",
	   device->CreateRenderTargetView(texture, &rtvDesc, &rtv));

	if ((autoGeneratedDesc.access & TextureAccess::READ) != 0) {
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;

		DX("Failed to create D3D11 SRV",
		   device->CreateShaderResourceView(texture, &srvDesc, &srv));
	}

	if ((autoGeneratedDesc.access & TextureAccess::WRITE) != 0) {
		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.Format = format;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		uavDesc.Texture2D.MipSlice = 0;

		DX("Failed to create D3D11 UAV",
		   device->CreateUnorderedAccessView(texture, &uavDesc, &uav));
	}

	return true;
}

void CD3D11to11SharedTexture::Destroy() {
	/**
	 * The texture we own is like a proxy to the shared texture. We still need
	 * to release it, but we don't own the shared texture.
	 */
	if (texture) {
		texture->Release();
		texture = nullptr;
	}

	if (srv) {
		srv->Release();
		srv = nullptr;
	}

	if (rtv) {
		rtv->Release();
		rtv = nullptr;
	}

	if (uav) {
		uav->Release();
		uav = nullptr;
	}
}

void CD3D11to11SharedTexture::AttachToContext(IRenderContext *context) {
	if (context->GetRenderAPI() != ContextRenderAPI::D3D11) {
		throw std::logic_error(
			"CD3D11to11SharedTexture::AttachToContext() encountered an "
			"unsupported render API"
		);
	}

	this->context = context;
}

void CD3D11to11SharedTexture::SetFullscreenSize() {
	throw std::logic_error(
		"CD3D11to11SharedTexture::SetFullscreenSize() should not be called"
	);
}

void *CD3D11to11SharedTexture::GetSharedHandle() {
	throw std::logic_error(
		"CD3D11to11SharedTexture::GetSharedHandle() should not be called"
	);
}

void *CD3D11to11SharedTexture::GetResource(TextureResource resource) {
	switch (resource) {
		case TextureResource::D3D11_SRV:
			return srv;
		case TextureResource::D3D11_RTV:
			return rtv;
		case TextureResource::D3D11_UAV:
			return uav;
		default:
			return nullptr;
	}
}