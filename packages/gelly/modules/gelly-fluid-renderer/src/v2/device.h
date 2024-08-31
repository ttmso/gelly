#ifndef DEVICE_H
#define DEVICE_H

#include <d3d11.h>
#include <d3d11_1.h>
#include <helpers/comptr.h>

namespace gelly {
namespace renderer {

/**
 * Encapsulates the D3D11 device and device context, and also exposes resource
 * management.
 */
class Device {
public:
	Device();
#ifdef GELLY_USE_NVAPI
	~Device();
#else
	~Device() = default;
#endif

	auto GetRawDevice() -> ComPtr<ID3D11Device>;
	auto GetRawDeviceContext() -> ComPtr<ID3D11DeviceContext>;
	auto GetPerformanceMarker() -> ComPtr<ID3DUserDefinedAnnotation>;
#ifdef GELLY_USE_NVAPI
	/**
	 * May be false if something went wrong during NVAPI initialization.
	 * @return true if NVAPI is available, false otherwise
	 */
	auto IsNVAPIAvailable() -> bool;
#endif
private:
	ComPtr<ID3D11Device> device;
	ComPtr<ID3D11DeviceContext> deviceContext;
	ComPtr<ID3DUserDefinedAnnotation> performanceMarker;

#ifdef GELLY_USE_NVAPI
	bool nvapiAvailable = false;
#endif

	auto CreateDevice(ComPtr<ID3D11Device> &device) -> void;
	auto QueryForPerformanceMarker(
		ComPtr<ID3DUserDefinedAnnotation> &performanceMarker
	) -> void;
	auto GetDeviceFlags() -> D3D11_CREATE_DEVICE_FLAG;
	auto GetFeatureLevel() -> D3D_FEATURE_LEVEL;
};

}  // namespace renderer
}  // namespace gelly

#endif	// DEVICE_H
