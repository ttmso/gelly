#ifndef SCENE_H
#define SCENE_H
// clang-format off
#include "Map.h"
// clang-format on

#include <GellyFluidSim.h>
#include <fluidsim/IFluidSimulation.h>

#include "Config.h"
#include "EntIndex.h"
#include "EntityManager.h"
#include "GarrysMod/Lua/SourceCompat.h"
#include "ParticleManager.h"
#include "fluidrender/IFluidRenderer.h"

class Scene {
private:
	std::shared_ptr<ISimContext> simContext;
	std::shared_ptr<IFluidSimulation> sim;
	std::shared_ptr<IFluidRenderer> connectedRenderer;

	std::optional<EntityManager> ents;
	std::optional<Map> map;
	ParticleManager particles;
	Config config;

public:
	Scene(
		const std::shared_ptr<IFluidRenderer> &connectedRenderer,
		const std::shared_ptr<ISimContext> &simContext,
		const std::shared_ptr<IFluidSimulation> &sim,
		int maxParticles
	);

	~Scene() = default;

	void AddEntity(EntIndex entIndex, std::vector<Vector> vertices);
	void AddPlayerObject(EntIndex entIndex, float radius, float halfHeight);
	void RemoveEntity(EntIndex entIndex);
	void UpdateEntityPosition(EntIndex entIndex, Vector position);
	void UpdateEntityRotation(EntIndex entIndex, XMFLOAT4 rotation);

	void LoadMap(const std::string &mapPath);

	void AddParticles(const ParticleListBuilder &builder) const;
	void ClearParticles() const;

	void SetFluidProperties(const SetFluidProperties &props) const;
	void ChangeRadius(float radius) const;

	[[nodiscard]] int GetActiveParticles() const {
		return sim->GetSimulationData()->GetActiveParticles();
	}

	[[nodiscard]] int GetMaxParticles() const {
		return sim->GetSimulationData()->GetMaxParticles();
	}

	[[nodiscard]] const char *GetComputeDevice() const {
		return sim->GetComputeDeviceName();
	}

	void Simulate(float dt) {
		sim->Update(dt);
		sim->GetScene()->Update();
	}

	void SetTimeStepMultiplier(float timeStepMultiplier) {
		sim->SetTimeStepMultiplier(fminf(timeStepMultiplier, 0.0001f));
	}
};

#endif	// SCENE_H
