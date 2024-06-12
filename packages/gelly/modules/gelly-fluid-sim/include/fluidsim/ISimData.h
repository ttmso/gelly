#ifndef GELLY_ISIMDATA_H
#define GELLY_ISIMDATA_H

#include "GellyInterface.h"

namespace Gelly {
struct SimFloat4 {
	float x, y, z, w;
};

enum class SimBufferType {
	POSITION,
	VELOCITY,
	FOAM_POSITION,
	FOAM_VELOCITY,
	// Basis vectors for oriented ellipsoid surface extraction
	ANISOTROPY_Q1,
	ANISOTROPY_Q2,
	ANISOTROPY_Q3
};
}  // namespace Gelly

using namespace Gelly;

gelly_interface ISimData {
public:
	/**
	 * Destroys the underlying buffers.
	 */
	virtual ~ISimData() = default;

	/**
	 * Sets the underlying pointer to a buffer resource in the same rendering
	 * API as the parent simulation is in.
	 *
	 * Basically, this function links a buffer to the simulation data, which is
	 * useful for rendering without having to perform CPU readbacks.
	 */
	virtual void LinkBuffer(SimBufferType type, void *buffer) = 0;
	virtual bool IsBufferLinked(SimBufferType type) = 0;

	virtual void *GetLinkedBuffer(SimBufferType type) = 0;
	virtual SimContextAPI GetAPI() = 0;

	virtual void SetMaxFoamParticles(int maxFoamParticles) = 0;
	virtual int GetMaxFoamParticles() = 0;

	virtual void SetActiveFoamParticles(int activeFoamParticles) = 0;
	virtual int GetActiveFoamParticles() = 0;

	virtual void SetMaxParticles(int maxParticles) = 0;
	virtual int GetMaxParticles() = 0;

	virtual void SetActiveParticles(int activeParticles) = 0;
	virtual int GetActiveParticles() = 0;

	// Scale divisors are a way to run a simulation in a local space (you'd
	// probably want to use this to lower a scale for better
	// performance/precision) and then scale the results back up to world space
	// opaquely.
	virtual float GetScaleDivisor() = 0;
	virtual void SetScaleDivisor(float scaleDivisor) = 0;
};

#endif	// GELLY_ISIMDATA_H
