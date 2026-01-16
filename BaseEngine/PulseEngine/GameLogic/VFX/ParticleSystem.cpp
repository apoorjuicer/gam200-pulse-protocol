#include "ParticleSystem.h"
#include <cstdlib>
#include <cmath>

float ParticleSystem::Rand01()
{
    return float(std::rand()) / float(RAND_MAX);
}

void ParticleSystem::CreatePool(std::size_t maxParticles)
{
    mParticles.assign(maxParticles, Particle{});
    mNextSearch = 0;
}

void ParticleSystem::Clear()
{
    // Pooling: do NOT clear the vector.
    for (auto& p : mParticles)
        p.active = false;

    mNextSearch = 0;
}

Particle* ParticleSystem::FindFree()
{
    if (mParticles.empty()) return nullptr;

    // Start searching from last free index to reduce scans
    for (std::size_t i = 0; i < mParticles.size(); ++i)
    {
        std::size_t idx = (mNextSearch + i) % mParticles.size();
        if (!mParticles[idx].active)
        {
            mNextSearch = (idx + 1) % mParticles.size();
            return &mParticles[idx];
        }
    }
    return nullptr; // pool full
}

void ParticleSystem::SpawnBurst(const ParticleSpawnDesc& d)
{
    for (int i = 0; i < d.count; ++i)
    {
        Particle* p = FindFree();
        if (!p) break;

        p->active = true;
        p->pos = d.position;

        float a = Rand01() * 6.2831853f;
        float s = d.speedMin + (d.speedMax - d.speedMin) * Rand01();
        p->vel = glm::vec2(std::cos(a), std::sin(a)) * s;

        p->lifeMax = d.lifeMin + (d.lifeMax - d.lifeMin) * Rand01();
        p->life = p->lifeMax;
        p->size = 1.0f;
    }
}

void ParticleSystem::Update(float dt)
{
    for (auto& p : mParticles)
    {
        if (!p.active) continue;

        p.life -= dt;
        if (p.life <= 0.0f)
        {
            p.active = false;
            continue;
        }

        p.pos += p.vel * dt;
    }
}
