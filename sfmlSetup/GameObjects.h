#pragma once
#include <iostream>
#include <vector>
#include <cfloat>
#include <algorithm>
#include <box2d/box2d.h>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <optional> // Assuming std::optional is used

namespace GameObjects
{
    struct Particle {
        b2BodyId bodyId;
        b2Circle shape;
        int proxyId;
        int life = -1 ;
        int age = 0;
        b2Vec2 pos = b2Vec2_zero;
        b2Vec2 LinearVelocity = b2Vec2_zero;
        b2Vec2 nextTickForce = b2Vec2_zero;
        b2Vec2 nextTickLinearImpulse = b2Vec2_zero;
        float nearDensity;
        float nearPressure;
    };
    struct ParticleConfig {
        float radius = 3.0f;
        float density = 2.5f;
        float friction = 0.0f;
        float restitution = 0.25f;
        float Impact = 4.f;
        float FORCE_MULTIPLIER = -2000.0f;
        float FORCE_SURFACE = 0.75f;
        float MomentumCoefficient = 1.f;
    };
    struct ParticleGroup{
        std::vector<GameObjects::Particle> Particles;
        GameObjects::ParticleConfig Config;
    };
    /*
    void addForceToParticle(b2Vec2 force, GameObjects::Particle* particle)
    {
        particle->nextTickForce += force;
        return;
    }*/
}