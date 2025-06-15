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


void ParticleGroup::UpdateData(World world) {
    for (auto& bucket : gridBuckets) {
        bucket.clear();
    }

    std::vector<int> toDelete;
    toDelete.reserve(Particles.size());

    for (int i = 0, n = int(Particles.size()); i < n; ++i) {
        Particle& p = Particles[i];

        if (p.life >= 0 && ++p.age > p.life) {
            toDelete.push_back(i);
            continue;
        }

        p.nextTickLinearImpulse = b2Vec2_zero;

        p.pos = b2Body_GetPosition(p.bodyId);
        p.LinearVelocity = b2Body_GetLinearVelocity(p.bodyId);

        int gx = std::clamp(int((p.pos.x + cellSize * gridSizeX * 0.5f) / cellSize), 0, gridSizeX - 1);
        int gy = std::clamp(int((p.pos.y + cellSize * gridSizeY * 0.5f) / cellSize), 0, gridSizeY - 1);
        gridBuckets[gy * gridSizeX + gx].push_back(&p);
    }

    for (int j = int(toDelete.size()) - 1; j >= 0; --j) {
        int idx = toDelete[j];
        b2DestroyBody(Particles[idx].bodyId);
        Particles.erase(Particles.begin() + idx);
    }
}

    void ParticleGroup::CreateParticle(GameObjects::World world,  float gravityScale, float radius, float x, float y, float density, float friction, float restitution,sf::Color color) {
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
        ///*
        {
            b2Filter filter;
            filter.categoryBits = 0x0002;
            filter.maskBits = 0xFFFF & ~(0x0002);
            shapeDef.filter = filter;
        }
        //*/
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
       // if ((int)world.clock.getElapsedTime().asSeconds() % 2 >= 1) {
            p.color = color;
        //}
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
            float x_pow2_75 = x_sq * MathUtils::Q_sqrt(x) * MathUtils::Q_sqrt(MathUtils::Q_sqrt(x));
			float radius_sq = radius * radius;
            return x_pow2_75 / (B2_PI * radius_sq * radius_sq / 4.0f);
        }


        /*
        float ParticleGroup::GetForce(float dst, float radius) {
            if (dst >= radius)
                return 0;
            float volume = (B2_PI * std::pow(radius, 4)) / 4;
            float x = radius - dst;
            return std::pow(radius - dst, 3.f) / volume;
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

            thread_local std::vector<Particle*> neighbors;
            for (int idx = start; idx < end; ++idx) {
                Particle& p = Particles[idx];
                if (b2Body_IsAwake(p.bodyId)) {
                    float range = p.shape.radius * Config.Impact;

                    neighbors.clear();
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

                                float dist = b2Length(other->pos - p.pos);
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


                    b2Vec2 posA = p.pos;
                    b2Vec2 posB = b2Vec2_zero;
                    b2Vec2 offset = b2Vec2_zero;
                    b2Vec2 velA = p.LinearVelocity;
                    float radiusA = p.shape.radius / (p.shape.radius / Config.Impact);
                    float radiusB = 0.f;
                    float dst = 0.f;

                    for (Particle* other : neighbors) {

                        //sf::Clock clock;

                        posB = other->pos;
                        radiusB = other->shape.radius / (p.shape.radius / Config.Impact);
                        offset = posB - posA;
                        dst = b2Length(offset) / (p.shape.radius / Config.Impact);
                        float effectiveRange = (radiusA + radiusB) * Config.Impact;
                        if (dst < effectiveRange) {
                            b2Vec2 forceDir = b2Normalize(offset);

                            float densityScale = std::clamp(density, 0.0f, 2.5f);
                            float distanceForceMag = GetForce(dst, effectiveRange) * densityScale * MathUtils::Q_sqrt(densityScale);

                            b2Vec2 repulsionForce = (Config.FORCE_MULTIPLIER) * forceDir * distanceForceMag * effectiveRange;

                            b2Vec2 velB = other->LinearVelocity;
                            b2Vec2 momentumForce = (velA - velB) * ((effectiveRange - dst) / effectiveRange) * Config.MomentumCoefficient;

                            neighborSum += posB;
                            attractCount++;
                            

                            b2Vec2 totalForce = repulsionForce + momentumForce;

                            float rNorm = dst / effectiveRange;
                            rNorm = std::clamp(rNorm, 0.1f, 1.0f); 

                            static auto ViscosityKernelSimple = [](float r)->float {
                                return (-0.5f * r * r * r + r * r - 1.0f);
                                };
                            float kVisc = ViscosityKernelSimple(rNorm);

                            b2Vec2 velDiff = other->LinearVelocity - p.LinearVelocity;
                            float maxVelDiff = 10.0f;
                            float len = b2Length(velDiff);
                            if (len > maxVelDiff) {
                                velDiff = velDiff * (maxVelDiff / len); 
                            }

                            float dot = b2Dot(velDiff, offset);
                            float leaveMul = (dot > 0 ? Config.VISCOSITY_LEAVE : 1.0f);

                            b2Vec2 viscForce = Config.VISCOSITY * leaveMul * kVisc * velDiff;


                            b2Vec2 dir = b2Normalize(offset);
                            b2Vec2 tangent = { -dir.y, dir.x }; 

                            float tangentialSpeed = b2Dot(other->LinearVelocity - p.LinearVelocity, tangent);
                            b2Vec2 frictionForce = -Config.SHEAR_VISCOSITY * tangentialSpeed * tangent;

                            float attenuation = 1.0f - (rNorm);
                            frictionForce *= attenuation;

                            totalForce += frictionForce;


                            totalForce += viscForce;

                            //totalForce *= (p.shape.radius / 2.0f);
                            {
                                std::lock_guard<std::mutex> lk(mutex);
                                other->nextTickLinearImpulse += totalForce;
                                p.nextTickLinearImpulse -= totalForce;

                                //sf::Time elapsed = clock.getElapsedTime();
                                //costtime += elapsed.asMilliseconds();
                            }
                        }
                    }

                    if (attractCount > 0) {
                        b2Vec2 avgPos = { neighborSum.x / float(attractCount), neighborSum.y / float(attractCount) };
                        p.nextTickLinearImpulse += Config.FORCE_SURFACE * (avgPos - p.pos);
                    }
                    if (b2Length(p.LinearVelocity) > 10 && p.neighborCount <= 5 && b2Length( p.nextTickLinearImpulse) > 80 )
                        p.color = sf::Color::White;
                    else {
                        p.color = sf::Color::Cyan;
                    }
                    /*
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

                        if ((nameA == "Particle" && nameB != "Particle") || (nameB == "Particle" && nameA != "Particle")) {
                            for (int k = 0; k < manifold.pointCount; ++k) {
                                b2Vec2 contactPoint = manifold.points[k].point;
                                b2BodyId otherBody = nameA == "Particle" ? bodyIdB : bodyIdA;
                                b2Vec2 normal = b2Normalize(contactPoint - p.pos);
                                b2Vec2 relativeVel = b2Body_GetLinearVelocity(otherBody) - p.LinearVelocity;

                                if (b2Dot(relativeVel, normal) > 0) {
                                    p.nextTickLinearImpulse += normal * Config.FORCE_ADHESION;
                                }

                            }
                        }
                    }*/

                }
                else {
                    p.nextTickLinearImpulse = b2Vec2_zero;
                }
            }
        }
        void ParticleGroup::ComputeParticleForces() {
            //costtime = 0.f;
            const int n = Particles.size();
            if (n == 0) return;

            int chunkSize = 256;
            if (n <= 500)
                chunkSize = 512;
            int t = std::min(std::thread::hardware_concurrency(), (unsigned int)std::ceil(n / chunkSize));
            static ThreadPool pool(t);
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
            //std::printf("ComputeParticleForces ºÄÊ±: %.f ms\n", costtime);
        }
}