#pragma once
#include <iostream>
#include <vector>
#include <cfloat>
#include <algorithm>
#include <box2d/box2d.h>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <optional> 

namespace GameObjects
{

    struct World
    {
        b2WorldId worldId;

    };

    struct Particle {
        b2BodyId bodyId;
        b2ShapeId shapeId;
        b2Circle shape;
        int proxyId;
        int life = -1 ;
        int age = 0;
        b2Vec2 pos = b2Vec2_zero;
        b2Vec2 LinearVelocity = b2Vec2_zero;
        b2Vec2 nextTickForce = b2Vec2_zero;
        b2Vec2 nextTickLinearImpulse = b2Vec2_zero;
        std::vector<b2JointId> AdhesionJoint;
    };


    struct ParticleConfig {
        float radius = 3.0f;
        float density = 2.5f;
        float friction = 0.0f;
        float restitution = 0.25f;
        float Impact = 3.25f;
        float FORCE_MULTIPLIER = -2000.0f;
        float FORCE_SURFACE = 1.5f;
        float FORCE_ADHESION = 0.05f;
        float MomentumCoefficient = 1.f;
    };


    struct ParticleGroup {
        void init();
        std::vector<GameObjects::Particle> Particles;
        GameObjects::ParticleConfig Config;
        b2DynamicTree dynamicTree;
        std::unordered_map<int, GameObjects::Particle*> proxyMap;

        void UpdateDynamicTree();
        void CreateParticle(GameObjects::World world, float radius, float x, float y, float density, float friction, float restitution);
        void DestroyParticle(GameObjects::World world, GameObjects::Particle* particle);
        static bool QueryCallback(int proxyId, int userData, void* context);
        float GetForce(float dst, float radius);
        void ComputeParticleForces();
    };

    struct QueryContext {
        GameObjects::Particle* current;
        std::vector<GameObjects::Particle*> neighbors;
        GameObjects::ParticleGroup* particleGroup;
    };
    /*
    void addForceToParticle(b2Vec2 force, GameObjects::Particle* particle)
    {
        particle->nextTickForce += force;
        return;
    }*/
}