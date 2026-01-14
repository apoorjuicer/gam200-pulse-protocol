#include "ParticleSystem.h"
#include <cstdlib>
#include <algorithm>
#include <cmath>

float ParticleSystem::Rand01()
{
    return float(std::rand()) / float(RAND_MAX);
}

void ParticleSystem::Reserve(size_t maxParticles)
{
    mParticles.reserve(maxParticles);
}

void ParticleSystem::Clear()
{
    mParticles.clear();
}

void ParticleSystem::SpawnBurst(const ParticleSpawnDesc& d)
{
    for (int i = 0; i < d.count; ++i)
    {
        Particle p;
        p.pos = d.position;

        float a = Rand01() * 6.2831853f; // 2pi
        float s = d.speedMin + (d.speedMax - d.speedMin) * Rand01();
        p.vel = glm::vec2(std::cos(a), std::sin(a)) * s;

        p.lifeMax = d.lifeMin + (d.lifeMax - d.lifeMin) * Rand01();
        p.life = p.lifeMax;

        p.size = 1.0f;

        mParticles.push_back(p);
    }
}

void ParticleSystem::Update(float dt)
{
    // update + remove dead (swap-pop)
    for (size_t i = 0; i < mParticles.size(); )
    {
        Particle& p = mParticles[i];
        p.life -= dt;

        if (p.life <= 0.0f)
        {
            mParticles[i] = mParticles.back();
            mParticles.pop_back();
            continue;
        }

        p.pos += p.vel * dt;
        ++i;
    }
}
