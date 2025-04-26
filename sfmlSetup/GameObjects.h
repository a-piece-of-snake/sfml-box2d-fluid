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
#include <cstdlib>
#include <cmath>
#include <mutex>
#include <box2d/collision.h>
#include <box2d/math_functions.h>
#include "GameObjects.h"
#include "MathUtils.h"
namespace GameObjects
{

    struct World
    {
        b2WorldId worldId;

    };

    struct Particle {
        int life = -1;
        int age = 0;
        float mass = 1.f;
        b2Vec2 pos = b2Vec2_zero;
        b2Vec2 LinearVelocity = b2Vec2_zero;
        b2Vec2 nextTickForce = b2Vec2_zero;
        b2Vec2 nextTickLinearImpulse = b2Vec2_zero;

        b2BodyId bodyId;
        b2ShapeId shapeId;
        b2Circle shape;
        int neighborCount = 0;
        int FreezeTime = 0;
        std::vector<b2JointId> AdhesionJoint;
        std::unordered_map<Particle*, float> restDistance;
    };


    struct ParticleConfig {
        float radius = 3.0f;
        float friction = 0.0f;
        float restitution = 0.25f;
        float Impact = 6.f;
        float FORCE_MULTIPLIER = -500.0f;
        float FORCE_SURFACE = 50.f;
        float FORCE_ADHESION = 100.f;
        float MomentumCoefficient = 1.f;
    };


    struct ParticleGroup {
        void init();
        std::vector<GameObjects::Particle> Particles;
        GameObjects::ParticleConfig Config;
        b2DynamicTree dynamicTree;
        std::vector<std::vector<Particle*>> gridBuckets;
        int gridSizeX = 5000; 
        int gridSizeY = 5000;
        float cellSize = 10.0f;

        int getGridIndex(int x, int y) const {
            x = std::clamp(x, 0, gridSizeX - 1);
            y = std::clamp(y, 0, gridSizeY - 1);
            return y * gridSizeX + x;
        }

        void clearGrid() {
            for (auto& bucket : gridBuckets) {
                bucket.clear();
            }
        }
        const int prime1 = 6614058611;
        const int prime2 = 7528850467;
        const int hashMapSize = 50000;

        int getHash2D(std::pair<int, int> gridPos);
        std::pair<int, int> getGridPos(GameObjects::Particle& pariticle);
        void UpdateData(GameObjects::World world);
        void CreateParticle(GameObjects::World world, float gravityScale, float radius, float x, float y, float density, float friction, float restitution);
        void DestroyParticle(GameObjects::World world, GameObjects::Particle* particle);
        float GetForce(float dst, float radius);
        void freeze();
        void unfreeze();
        void ComputeChunkForces(int start, int end);
        void ComputeParticleForces();
		std::mutex mutex;
    };
}