#ifndef GELLY_MATHTYPES_H
#define GELLY_MATHTYPES_H

// gives us Vector and QAngle
#include <GarrysMod/Lua/SourceCompat.h>

/**
 * @brief A 4x4 matrix, used by Source.
 * @note If this is required in a D3D9 context, you can just cast this to a
 * D3DMATRIX.
 */
struct VMatrix {
	float m[4][4];
};

#endif	// GELLY_MATHTYPES_H
