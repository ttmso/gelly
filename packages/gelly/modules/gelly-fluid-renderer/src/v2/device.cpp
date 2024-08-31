#include "device.h"

#include <helpers/throw-informative-exception.h>

#include <stdexcept>

#ifdef GELLY_USE_NVAPI
#include <nvShaderExtnEnums.h>
#include <nvapi.h>

#include "nvapi-extension-slot.h"
#endif

namespace gelly {
namespace renderer {

Device::Device() :
	device(nullptr), deviceContext(nullptr), performanceMarker(nullptr) {
	CreateDevice(device);
	QueryForPerformanceMarker(performanceMarker);
}

#ifdef GELLY_USE_NVAPI
Device::~Device() {
	if (NvAPI_Unload() != NVAPI_OK) {
		printf("[Device::~Device] Failed to unload NVAPI\n");
	}
}
#endif

auto Device::GetRawDevice() -> ComPtr<ID3D11Device> { return device; }
auto Device::GetRawDeviceContext() -> ComPtr<ID3D11DeviceContext> {
	return deviceContext;
}
auto Device::GetPerformanceMarker() -> ComPtr<ID3DUserDefinedAnnotation> {
	return performanceMarker;
}

#ifdef GELLY_USE_NVAPI
auto Device::IsNVAPIAvailable() -> bool { return nvapiAvailable; }
#endif

auto Device::CreateDevice(ComPtr<ID3D11Device> &device) -> void {
#ifdef GELLY_USE_NVAPI
	if (const auto status = NvAPI_Initialize(); status != NVAPI_OK) {
		GELLY_RENDERER_THROW(std::runtime_error, "Failed to initialize NVAPI!");
	} else {
		// typically when we're using NVAPI we want to log each and every
		// detail so we can see what's going on
		printf("[Device::CreateDevice] NVAPI initialized\n");
		nvapiAvailable = true;
	}

	auto chipsetInfo = NV_CHIPSET_INFO{};
	chipsetInfo.version = NV_CHIPSET_INFO_VER;

	NvU32 driverVersion = 0;
	NvAPI_ShortString driverVersionString = {};

	if (NvAPI_SYS_GetChipSetInfo(&chipsetInfo) == NVAPI_OK &&
		NvAPI_SYS_GetDriverAndBranchVersion(
			&driverVersion, driverVersionString
		) == NVAPI_OK) {
		printf(
			"[Device::CreateDevice] Gelly running on %s %s (%lu) | (driver "
			"v%lu at branch %s)\n",
			chipsetInfo.szVendorName,
			chipsetInfo.szChipsetName,
			chipsetInfo.deviceId,
			driverVersion,
			driverVersionString
		);
	}
#endif

	auto featureLevel = GetFeatureLevel();
#ifdef GELLY_USE_NVAPI
	auto nvapiFeatureLevel = NVAPI_DEVICE_FEATURE_LEVEL_NULL;
	const auto result = NvAPI_D3D11_CreateDevice(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		GetDeviceFlags(),
		&featureLevel,
		1,
		D3D11_SDK_VERSION,
		device.GetAddressOf(),
		&featureLevel,
		deviceContext.GetAddressOf(),
		&nvapiFeatureLevel
	);
#else
	const auto result = D3D11CreateDevice(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		GetDeviceFlags(),
		nullptr,
		0,
		D3D11_SDK_VERSION,
		device.GetAddressOf(),
		&featureLevel,
		deviceContext.GetAddressOf()
	);
#endif

	if (FAILED(result)) {
		GELLY_RENDERER_THROW(
			std::runtime_error, "Failed to create D3D11 device!"
		);
	}

#ifdef GELLY_USE_NVAPI
	ComPtr<IUnknown> nvAPIDevice;
	if (device.As(&nvAPIDevice) != S_OK) {
		GELLY_RENDERER_THROW(
			std::runtime_error, "Failed to get NVAPI device from D3D11 device"
		);
	}

	bool isGetSpecialSupported = false;
	if (NvAPI_D3D11_IsNvShaderExtnOpCodeSupported(
			nvAPIDevice.Get(), NV_EXTN_OP_GET_SPECIAL, &isGetSpecialSupported
		) != NVAPI_OK) {
		GELLY_RENDERER_THROW(
			std::runtime_error,
			"Failed to check if NVAPI extension is supported"
		);
	}

	if (!isGetSpecialSupported) {
		GELLY_RENDERER_THROW(
			std::runtime_error,
			"NVAPI's GetSpecial is not supported on this device"
		);
	}

	if (NvAPI_D3D11_SetNvShaderExtnSlot(
			nvAPIDevice.Get(), INSTR_EXTENSION_UAV_SLOT
		) != NVAPI_OK) {
		GELLY_RENDERER_THROW(
			std::runtime_error, "Failed to set NVAPI extension slot"
		);
	} else {
		printf(
			"[Device::CreateDevice] NVAPI extension slot set to %lu\n",
			INSTR_EXTENSION_UAV_SLOT
		);
	}
#endif
}

auto Device::QueryForPerformanceMarker(
	ComPtr<ID3DUserDefinedAnnotation> &performanceMarker
) -> void {
	if (FAILED(deviceContext->QueryInterface(
			IID_PPV_ARGS(performanceMarker.GetAddressOf())
		))) {
		GELLY_RENDERER_THROW(
			std::runtime_error, "Failed to create perf marker"
		);
	}
}

auto Device::GetFeatureLevel() -> D3D_FEATURE_LEVEL {
	return D3D_FEATURE_LEVEL_11_1;
}

auto Device::GetDeviceFlags() -> D3D11_CREATE_DEVICE_FLAG {
	return D3D11_CREATE_DEVICE_SINGLETHREADED;
}

}  // namespace renderer
}  // namespace gelly