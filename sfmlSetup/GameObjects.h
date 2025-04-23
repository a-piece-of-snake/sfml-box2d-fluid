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
#include "ThreadPool.h"
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
        int life = -1;
        int age = 0;
		float mass = 1.f;
        int neighborCount = 0;
        int FreezeTime = 0;
        b2Vec2 pos = b2Vec2_zero;
        b2Vec2 LinearVelocity = b2Vec2_zero;
        b2Vec2 nextTickForce = b2Vec2_zero;
        b2Vec2 nextTickLinearImpulse = b2Vec2_zero;
        std::vector<b2JointId> AdhesionJoint;
        std::unordered_map<Particle*, float> restDistance;
        bool operator==(const Particle& other) const {
            return bodyId.index1 == other.bodyId.index1 &&
                shapeId.index1 == other.shapeId.index1 &&
                proxyId == other.proxyId;
        }
    };


    struct ParticleConfig {
        float radius = 3.0f;
        float friction = 0.0f;
        float restitution = 0.25f;
        float Impact = 3.f;
        float FORCE_MULTIPLIER = -500.0f;
        float FORCE_SURFACE = 10.f;
        float FORCE_ADHESION = 100.f;
        float MomentumCoefficient = 1.f;
        float PLASTICITY_THRESHOLD = 0.2f;   // 相对应变阈值（20%）
        float PLASTICITY_RATE = 0.1f;
    };


    struct ParticleGroup {
        void init();
        std::vector<GameObjects::Particle> Particles;
        GameObjects::ParticleConfig Config;
        b2DynamicTree dynamicTree;
        std::unordered_map<int, GameObjects::Particle*> proxyMap;

        ThreadPool* pool = nullptr;
        void UpdateDynamicTree();
        void CreateParticle(GameObjects::World world, float gravityScale, float radius, float x, float y, float density, float friction, float restitution);
        void DestroyParticle(GameObjects::World world, GameObjects::Particle* particle);
        static bool QueryCallback(int proxyId, int userData, void* context);
        float GetForce(float dst, float radius);
        void freeze();
        void unfreeze();
        void ComputeChunkForces(int start, int end);
        void ComputeParticleForces();
		std::mutex mutex;
    };

    struct QueryContext {
        GameObjects::Particle* current;
        std::vector<GameObjects::Particle*> neighbors;
        GameObjects::ParticleGroup* particleGroup;
		float density = 0.f;
    };
    /*
    void addForceToParticle(b2Vec2 force, GameObjects::Particle* particle)
    {
        particle->nextTickForce += force;
        return;
    }*/
}