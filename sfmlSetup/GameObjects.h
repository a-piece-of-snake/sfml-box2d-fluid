#pragma once
#include <iostream>
#include <vector>
#include <cfloat>
#include <algorithm>
#include <xmmintrin.h>
#include <optional> 
#include <cstdlib>
#include <cmath>
#include <mutex>
#include <omp.h>

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>

#include <box2d/box2d.h>
#include <box2d/collision.h>
#include <box2d/math_functions.h>

#include "ThreadPool.h"
#include "MathUtils.h"

namespace GameObjects
{
    //const float G = 6.6743f;
    struct World
    {
        b2WorldId worldId;
        sf::Clock clock;
        b2JointId mouseJointId = b2_nullJointId;
        b2BodyId groundBodyId;
		float timeStep;
		int subStep;
    };

    struct SpawnableObject {
        std::string name;
        std::string type;
        std::string describe;
        sf::Texture icon;
		unsigned int ID;
        std::function<bool(b2Vec2 pos)> onSpawned;
        bool spawn(b2Vec2 pos);
    };

    struct SpawnableObjectBrowser {
        std::vector<SpawnableObject> objects;
		float iconSize = 50.f;
        
		char searchBuffer[1024];

        std::string search;

        void addObject(SpawnableObject obj) {
            objects.emplace_back(std::move(obj));
		}

        void clearObjects() {
            objects.clear();
        }

    };


    struct Particle {
        int index;
        int life = -1;
        int age = 0;
        float mass = 1.f;
        b2Vec2 pos = b2Vec2_zero;
        b2Vec2 LinearVelocity = b2Vec2_zero;
        b2Vec2 nextTickForce = b2Vec2_zero;
        b2Vec2 nextTickLinearImpulse = b2Vec2_zero;
		unsigned int bubleTime = 0;

        sf::Color color = sf::Color::Cyan;
        b2BodyId bodyId;
        b2ShapeId shapeId;
        b2Circle shape;
        int neighborCount = 0;
        int FreezeTime = 0;
        std::vector<b2JointId> AdhesionJoint;
    };


    struct ParticleConfig {
        float radius = 4.f;
        float friction = 0.0f;
        float restitution = 0.01f;
        float Impact = 5.f;
        float MomentumCoefficient = 1.f;
        float FORCE_MULTIPLIER = 15000.f;
        float FORCE_SURFACE = 75.f;
        float FORCE_ADHESION = 0.f;
        float SHEAR_VISCOSITY = 20.f;
        float VISCOSITY = 8.f; 
        float VISCOSITY_LEAVE = 0.8f;  
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
        void CreateParticle(GameObjects::World world, float gravityScale, float radius, float x, float y, float density, float friction, float restitution,sf::Color color);
        void DestroyParticle(GameObjects::World world, GameObjects::Particle* particle);
        float GetForce(float dst, float radius);
        void freeze();
        void unfreeze();
        void ComputeChunkForces(int start, int end, int threadID, float timestep);
        void ApplyForce(int start, int end);
        void ComputeParticleForces(float timestep);
		std::mutex mutex;
    };
}