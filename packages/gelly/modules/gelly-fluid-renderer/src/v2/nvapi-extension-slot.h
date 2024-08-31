#ifndef NVAPI_EXTENSION_SLOT_H
#define NVAPI_EXTENSION_SLOT_H

#include <nvapi.h>

#ifndef GELLY_USE_NVAPI
#error "Cannot include nvapi-extension-slot.h without GELLY_USE_NVAPI defined"
#endif

namespace gelly {
namespace renderer {
constexpr NvU32 INSTR_EXTENSION_UAV_SLOT = 8;
}
}  // namespace gelly
#endif	// NVAPI_EXTENSION_SLOT_H
