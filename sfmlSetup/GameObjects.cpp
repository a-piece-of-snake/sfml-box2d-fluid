#include <iostream>
#include <vector>
#include <cfloat>
#include <algorithm>
#include <box2d/box2d.h>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <functional>
#include <thread>
#include <optional> 
#include <cstdlib>
#include <cmath>
#include <mutex>
#include <future>
#include <box2d/collision.h>
#include <box2d/math_functions.h>
#include "GameObjects.h"
#include "MathUtils.h"
#include "ThreadPool.h"

namespace GameObjects
{
    int ParticleGroup::getHash2D(std::pair<int, int> gridPos) {
        return hashMapSize / 2 + ((gridPos.first * prime1) ^ (gridPos.second * prime2)) % (hashMapSize / 2);
    }

    std::pair<int, int> ParticleGroup::getGridPos(GameObjects::Particle& pariticle) {
        int x = std::floor(pariticle.pos.x / (Config.radius * Config.Impact));
        int y = std::floor(pariticle.pos.y / (Config.radius * Config.Impact));
		return { x, y };
    }
    void ParticleGroup::init() {
        cellSize = Config.radius * Config.Impact * 2.0f;
        gridSizeX = static_cast<int>(10000.0f / cellSize); 
        gridSizeY = static_cast<int>(10000.0f / cellSize); 
        gridBuckets.resize(gridSizeX * gridSizeY);
        Particles.reserve(10000);
        Particles.clear();
        Particles.shrink_to_fit();
    }

    void ParticleGroup::UpdateData(GameObjects::World world) {
        for (auto& bucket : gridBuckets) {
            bucket.clear();
            bucket.reserve(8);
        }


        for (auto& p : Particles) {
            int gridX = static_cast<int>((p.pos.x + 5000.0f) / cellSize); 
            int gridY = static_cast<int>((p.pos.y + 5000.0f) / cellSize);
            gridX = std::clamp(gridX, 0, gridSizeX - 1);
            gridY = std::clamp(gridY, 0, gridSizeY - 1);
            gridBuckets[gridY * gridSizeX + gridX].push_back(&p);

            p.nextTickLinearImpulse = b2Vec2_zero;
            p.nextTickForce = b2Vec2_zero;
            p.pos = b2Body_GetPosition(p.bodyId);
            p.LinearVelocity = b2Body_GetLinearVelocity(p.bodyId);

            if (p.age >= p.life && p.life >= 0)
                DestroyParticle(world, &p);
            p.age++;
        }
    }


    void ParticleGroup::CreateParticle(GameObjects::World world,  float gravityScale, float radius, float x, float y, float density, float friction, float restitution) {
        Config.radius = radius;
        b2BodyDef bodyDef = b2DefaultBodyDef();
        bodyDef.position = { x, y };
        bodyDef.type = b2_dynamicBody;
        bodyDef.linearDamping = 0.01f;
        bodyDef.angularDamping = 0.01f;
        bodyDef.fixedRotation = true;
        bodyDef.isBullet = false;
        bodyDef.name = "Particle";
        bodyDef.gravityScale = gravityScale;
        //bodyDef.sleepThreshold = radius / 5.f;
        b2BodyId bodyId = b2CreateBody(world.worldId, &bodyDef);

        b2ShapeDef shapeDef = b2DefaultShapeDef();
        shapeDef.density = density;
        shapeDef.friction = friction;
        shapeDef.restitution = restitution;
        /*
        {
            b2Filter filter;
            filter.categoryBits = 0x0002;
            filter.maskBits = 0xFFFF & ~(0x0002);
            shapeDef.filter = filter;
        }
        */
        b2Circle circle;
        circle.radius = radius;
        circle.center = b2Vec2{ 0.0f, 0.0f };
        b2ShapeId shapeId = b2CreateCircleShape(bodyId, &shapeDef, &circle);

        const float queryRange = radius;
        //const float queryRange = radius * Config.Impact;
        b2AABB aabb;
        aabb.lowerBound = b2Vec2{ x - queryRange, y - queryRange };
        aabb.upperBound = b2Vec2{ x + queryRange, y + queryRange };

        GameObjects::Particle p;
        p.bodyId = bodyId;
        p.shape = circle;
        p.shapeId = shapeId;
        p.mass = b2Body_GetMass(p.bodyId);
        int index = Particles.size();
        Particles.push_back(p);
    }
        void ParticleGroup::DestroyParticle(GameObjects::World world, GameObjects::Particle* particle) {
            for (auto it = Particles.begin(); it != Particles.end(); ) {
                if (it->bodyId.generation == particle->bodyId.generation && it->bodyId.index1 == particle->bodyId.index1 && it->bodyId.world0 == particle->bodyId.world0) {
                    b2DestroyBody(it->bodyId);
                    it = Particles.erase(it);
                }
            }
        }
        float ParticleGroup::GetForce(float dst, float radius) {
            if (dst >= radius) return 0;
            float x = radius - dst;
            float x_sq = x * x;
            float x_pow2_75 = x_sq * std::sqrt(x) * std::sqrt(std::sqrt(x));
            return x_pow2_75 / (B2_PI * radius * radius * radius * radius / 4.0f);
        }
        /*
        float ParticleGroup::GetForce(float dst, float radius) {
            if (dst >= radius)
                return 0;
            float volume = (B2_PI * std::pow(radius, 4)) / 4;
            float x = radius - dst;
            return std::pow(radius - dst, 2.75f) / volume;
        }
        */


        void ParticleGroup::freeze() {
            for (auto& p : Particles) {
                std::vector<b2ContactData> contactData;
                int capacity = b2Shape_GetContactCapacity(p.shapeId);
                contactData.resize(capacity);
                int count = b2Body_GetContactData(p.bodyId, contactData.data(), capacity);
                for (int i = 0; i < count; ++i) {
                    b2Manifold manifold = contactData[i].manifold;

                    b2BodyId bodyIdA = b2Shape_GetBody(contactData[i].shapeIdA);
                    b2BodyId bodyIdB = b2Shape_GetBody(contactData[i].shapeIdB);
                    std::string nameA = b2Body_GetName(bodyIdA);
                    std::string nameB = b2Body_GetName(bodyIdB);
                    if (!(nameA == "Particle" && nameB == "Particle") && (nameA == "Particle" || nameB == "Particle")) {
                        for (int k = 0; k < manifold.pointCount; ++k) {
                            if (b2Body_GetType(p.bodyId) == b2_dynamicBody) {
                                b2Body_SetType(p.bodyId, b2_staticBody);
                            }
                            break;
                        }
                    }
                }
            }
        }

        void ParticleGroup::unfreeze() {
            for (auto& p : Particles) {
                if (b2Body_GetType(p.bodyId) == b2_staticBody) {
                    b2Body_SetType(p.bodyId, b2_dynamicBody);
                }
            }
        }
        void ParticleGroup::ComputeChunkForces(int start, int end) {
            for (int idx = start; idx < end; ++idx) {
                Particle& p = Particles[idx];
                if(b2Body_IsAwake(p.bodyId)) {
                    float range = p.shape.radius * Config.Impact;

                    std::vector<Particle*> neighbors;
                    neighbors.reserve(64);
                    float density = 0.0f;

                    int centerX = static_cast<int>((p.pos.x + 5000.0f) / cellSize);
                    int centerY = static_cast<int>((p.pos.y + 5000.0f) / cellSize);

                    for (int dx = -1; dx <= 1; ++dx) {
                        for (int dy = -1; dy <= 1; ++dy) {
                            int x = centerX + dx;
                            int y = centerY + dy;

                            if (x < 0 || x >= gridSizeX || y < 0 || y >= gridSizeY)
                                continue;

                            const auto& bucket = gridBuckets[y * gridSizeX + x];
                            for (Particle* other : bucket) {
                                if (other == &p) continue;

                                float dist = MathUtils::b2Vec2Length(other->pos - p.pos);
                                if (dist < range) {
                                    float influence = GetForce(dist, range);
                                    density += p.mass * influence;
                                    neighbors.push_back(other);
                                }
                            }
                        }
                    } p.neighborCount = int(neighbors.size());

                    b2Vec2 neighborSum = b2Vec2_zero;
                    int attractCount = 0;
                    for (Particle* other : neighbors) {
                        b2Vec2 posA = p.pos;
                        b2Vec2 posB = other->pos;
                        float radiusA = p.shape.radius / (p.shape.radius / Config.Impact);
                        float radiusB = other->shape.radius / (p.shape.radius / Config.Impact);
                        b2Vec2 offset = posB - posA;
                        float dst = MathUtils::b2Vec2Length(offset) / (p.shape.radius / Config.Impact);
                        float effectiveRange = (radiusA + radiusB) * Config.Impact;
                        if (dst < effectiveRange) {
                            dst = dst - radiusA;
                            b2Vec2 forceDir = MathUtils::b2Vec2Normalized(offset);
                            float distanceForceMag = GetForce(dst / std::max(density, 1e-3f), effectiveRange);
                            b2Vec2 repulsionForce = (-Config.FORCE_MULTIPLIER) * forceDir * distanceForceMag * effectiveRange;

                            b2Vec2 velA = p.LinearVelocity;
                            b2Vec2 velB = other->LinearVelocity;
                            b2Vec2 momentumForce = (velA - velB)
                                * ((effectiveRange - dst) / effectiveRange)
                                * Config.MomentumCoefficient;

                            float d0 = p.shape.radius * Config.Impact;
                            b2Vec2 springForce = b2Vec2_zero;
                            neighborSum += posB;
                            attractCount++;

                            b2Vec2 totalForce = repulsionForce + momentumForce + springForce;
                            totalForce *= (p.shape.radius / 2.0f);

                            {
                                std::lock_guard<std::mutex> lk(mutex);
                                other->nextTickLinearImpulse += totalForce;
                                p.nextTickLinearImpulse -= totalForce;
                            }
                        }
                    }

                    if (attractCount > 0) {
                        b2Vec2 avgPos = { neighborSum.x / float(attractCount), neighborSum.y / float(attractCount) };
                        p.nextTickLinearImpulse += Config.FORCE_SURFACE * (avgPos - p.pos);
                    }
                }
                else {
                    p.nextTickLinearImpulse = b2Vec2_zero;
                }
            }
        }
        void ParticleGroup::ComputeParticleForces() {
            const int n = Particles.size();
            if (n == 0) return;

            static ThreadPool pool(std::thread::hardware_concurrency());

            const int chunkSize = 250;
            std::vector<std::future<void>> futures;

            for (int start = 0; start < n; start += chunkSize) {
                int end = std::min(start + chunkSize, n);
                futures.push_back(
                    pool.enqueue([this, start, end]() { 
                        ComputeChunkForces(start, end);
                        }) 
                );
            }

            for (auto& f : futures) {
                f.wait();
            }
        }
}