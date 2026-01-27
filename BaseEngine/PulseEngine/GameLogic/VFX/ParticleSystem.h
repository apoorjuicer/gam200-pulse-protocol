#pragma once
#include <vector>
#include <glm/vec2.hpp>
#include <cstddef>

struct Particle
{
    bool active = false;
    glm::vec2 pos{ 0,0 };
    glm::vec2 vel{ 0,0 };
    float life = 0.0f;
    float lifeMax = 0.0f;
    float size = 1.0f;
};

struct ParticleSpawnDesc
{
    glm::vec2 position{ 0,0 };
    int count = 12;
    float speedMin = 0.3f;
    float speedMax = 1.2f;
    float lifeMin = 0.25f;
    float lifeMax = 0.6f;
};

class ParticleSystem
{
public:
    void CreatePool(size_t maxParticles);
    void Clear();

    void SpawnBurst(const ParticleSpawnDesc& d);
    void Update(float dt);

    const std::vector<Particle>& GetParticles() const { return mParticles; }

private:
    std::vector<Particle> mParticles;

    float Rand01();
    Particle* FindFree();
};
