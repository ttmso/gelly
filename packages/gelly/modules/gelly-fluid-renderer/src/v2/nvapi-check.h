#ifndef NVAPI_CHECK_H
#define NVAPI_CHECK_H

#include <nvapi.h>

#include <stdexcept>
#include <string>

#include "helpers/throw-informative-exception.h"

#ifndef GELLY_USE_NVAPI
#error "Cannot include nvapi-check.h without GELLY_USE_NVAPI defined"
#endif

namespace gelly {
namespace renderer {
inline auto CheckNVAPICall(NvAPI_Status status, const std::string &context)
	-> void {
	if (status != NVAPI_OK) {
		NvAPI_ShortString statusString;
		NvAPI_GetErrorMessage(status, statusString);

		GELLY_RENDERER_THROW(
			std::runtime_error,
			"NVAPI call failed with message: '" + std::string(statusString) +
				"' during context: " + context
		);
	}
}
}  // namespace renderer
}  // namespace gelly

#endif	// NVAPI_CHECK_H
