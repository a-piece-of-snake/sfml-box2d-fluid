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
#include <box2d/box2d.h>
#include <box2d/collision.h>
#include <box2d/math_functions.h>
#include "GameObjects.h"
#include "MathUtils.h"

namespace GameObjects
{
    void ParticleGroup::init() {
        dynamicTree = b2DynamicTree_Create();
    }

    void ParticleGroup::UpdateDynamicTree() {
        for (auto& p : Particles) {
            b2Vec2 pos = b2Body_GetPosition(p.bodyId);
            float radius = p.shape.radius;
            const float queryRange = radius * Config.Impact;
            b2AABB newAABB;
            newAABB.lowerBound = b2Vec2{ pos.x - queryRange, pos.y - queryRange };
            newAABB.upperBound = b2Vec2{ pos.x + queryRange, pos.y + queryRange };
            b2DynamicTree_MoveProxy(&dynamicTree, p.proxyId, newAABB);
        }
    }

    void ParticleGroup::CreateParticle(GameObjects::World world,  float gravityScale, float radius, float x, float y, float density, float friction, float restitution) {
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
        p.proxyId = b2DynamicTree_CreateProxy(&dynamicTree, aabb, B2_DEFAULT_CATEGORY_BITS, index);
        Particles.push_back(p);
        proxyMap[p.proxyId] = &Particles.back();
    }
        void ParticleGroup::DestroyParticle(GameObjects::World world, GameObjects::Particle* particle) {
            for (auto it = Particles.begin(); it != Particles.end(); ) {
                if (it->bodyId.generation == particle->bodyId.generation && it->bodyId.index1 == particle->bodyId.index1 && it->bodyId.world0 == particle->bodyId.world0) {
                    b2DestroyBody(it->bodyId);
                    b2DynamicTree_DestroyProxy(&dynamicTree, it->proxyId);
                    proxyMap.erase(it->proxyId);
                    it = Particles.erase(it);
                }
            }
        }
        float ParticleGroup::GetForce(float dst, float radius) {
            if (dst >= radius)
                return 0;
            float volume = (B2_PI * std::pow(radius, 4)) / 4;
            return std::pow(radius - dst, 2.75f) / volume;
        }


        bool ParticleGroup::QueryCallback(int proxyId, int userData, void* context) {
            GameObjects::QueryContext* ctx = static_cast<GameObjects::QueryContext*>(context);
            GameObjects::ParticleGroup* particleGroup = ctx->particleGroup;

            if (particleGroup->proxyMap.find(proxyId) == particleGroup->proxyMap.end())
                return true;

            int index = userData;
            if (index < 0 || index >= static_cast<int>(particleGroup->Particles.size()))
                return true;

            GameObjects::Particle* other = &particleGroup->Particles[index];
            if (other == ctx->current)
                return true;

            const float mass = ctx->current->mass;
			float dst = MathUtils::b2Vec2Length(other->pos - ctx->current->pos);
			float effectiveRange = (ctx->current->shape.radius + other->shape.radius) * ctx->particleGroup->Config.Impact;
            float influence = ctx->particleGroup->GetForce(dst, effectiveRange);
            ctx->density += mass * influence;
            ctx->neighbors.push_back(other);
            return true;
        }

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
                            /*
                            b2Circle circle = p.shape;
                            circle.radius *= Config.Impact;
                            p.shape = circle;
                            b2Shape_SetCircle(p.shapeId, &circle);
                            */
                            if (b2Body_GetType(p.bodyId) == b2_dynamicBody) {
                                b2Body_SetType(p.bodyId, b2_staticBody);
                            }
                            break;
                        }
                    }
                }
                /*
                // 停止粒子运动
                b2Body_SetLinearVelocity(p.bodyId, b2Vec2_zero);
                b2Body_SetAngularVelocity(p.bodyId, 0.0f);

                QueryContext ctx;
                ctx.current = &p;
                ctx.particleGroup = this;

                float influenceRange = p.shape.radius * (Config.Impact + 0.1f);
                b2AABB queryAABB;
                queryAABB.lowerBound = p.pos - b2Vec2{ influenceRange, influenceRange };
                queryAABB.upperBound = p.pos + b2Vec2{ influenceRange, influenceRange };
                b2DynamicTree_Query(&dynamicTree, queryAABB, B2_DEFAULT_MASK_BITS, QueryCallback, &ctx);

                for (auto* other : ctx.neighbors) {
                    b2ShapeDef shapeDef = b2DefaultShapeDef();
                    b2CreateCircleShape(p.bodyId, &shapeDef, &other->shape);
                }
            */
            }
        }

        void ParticleGroup::unfreeze() {
            for (auto& p : Particles) {
                if (b2Body_GetType(p.bodyId) == b2_staticBody) {
                    /*
                    b2Circle circle = p.shape;
                    circle.radius /= Config.Impact;
                    p.shape = circle;
                    b2Shape_SetCircle(p.shapeId, &circle);
                    */
                    b2Body_SetType(p.bodyId, b2_dynamicBody);
                }
            /*
                for (auto jointId : p.AdhesionJoint) {
                    b2DestroyJoint(jointId);
                }
                p.AdhesionJoint.clear();
            */
            }
        }
        void ParticleGroup::ComputeChunkForces(int start, int end) {
            for (int idx = start; idx < end; ++idx) {
                Particle& p = Particles[idx];
                QueryContext ctx;
                ctx.current = &p;
                ctx.particleGroup = this; b2Vec2 posA = p.pos;
                float influenceRange = p.shape.radius * Config.Impact;
                b2AABB queryAABB;
                queryAABB.lowerBound = posA - b2Vec2{ influenceRange, influenceRange };
                queryAABB.upperBound = posA + b2Vec2{ influenceRange, influenceRange };
                b2DynamicTree_Query(&dynamicTree, queryAABB, B2_DEFAULT_MASK_BITS, QueryCallback, &ctx);
                p.neighborCount = ctx.neighbors.size();
                float radiusA = p.shape.radius / (p.shape.radius / Config.Impact);

                bool havefreezep = false;
                b2Vec2 neighborSum = b2Vec2_zero;
                int attractCount = 0; 

                for (auto* other : ctx.neighbors) {
                    if (b2Body_GetType(other->bodyId) == b2_staticBody) {
                        p.FreezeTime++;
                        havefreezep = true;
                    }
                    if (b2Body_GetType(p.bodyId) == b2_staticBody) {
                        p.FreezeTime = 30;
                        havefreezep = true;
                    }
                    else if (p.FreezeTime >= 30) {
                        /*
                        b2Circle circle = p.shape;
                        circle.radius *= Config.Impact;
                        p.shape = circle;
                        b2Shape_SetCircle(p.shapeId, &circle);
                        */
                        b2Body_SetType(p.bodyId, b2_staticBody);
                    }
                    b2Vec2 posB = other->pos;
                    float radiusB = other->shape.radius / (p.shape.radius / Config.Impact);
                    b2Vec2 offset = posB - posA;
                    float dst = MathUtils::b2Vec2Length(offset) / (p.shape.radius / Config.Impact);
                    float effectiveRange = (radiusA + radiusB) * Config.Impact;
                    if (dst < effectiveRange) {
                        dst = dst - radiusA;
                        b2Vec2 forceDir = MathUtils::b2Vec2Normalized(offset);
                        float distanceForceMag = GetForce(dst / ctx.density, effectiveRange);
                        b2Vec2 repulsionForce = (-Config.FORCE_MULTIPLIER) * forceDir * distanceForceMag * effectiveRange;
                        b2Vec2 velA = p.LinearVelocity;
                        b2Vec2 velB = other->LinearVelocity;
                        b2Vec2 momentumForce = (velA - velB) * ((effectiveRange - dst) / effectiveRange) * Config.MomentumCoefficient;
                        float d0 = p.shape.radius * Config.Impact;
                        b2Vec2 springForce = b2Vec2_zero;

                        if (MathUtils::b2Vec2Length(offset) > d0) {
                            neighborSum += posB;
                            attractCount++;
                        }


                        b2Vec2 totalForce = repulsionForce + momentumForce + springForce;
                        totalForce *= (p.shape.radius / 2.0f);
                        mutex.lock();
                        other->nextTickLinearImpulse += totalForce;
                        p.nextTickLinearImpulse -= totalForce;
						mutex.unlock();
                    }
                }
                if (!havefreezep) {
                    p.FreezeTime = std::max(p.FreezeTime-1, 0);
                }
                if (attractCount > 0) {
                    b2Vec2 avgPos = { neighborSum.x / attractCount, neighborSum.y/ attractCount };
                    p.nextTickLinearImpulse += Config.FORCE_SURFACE * (avgPos - posA);
                    //std::cout << Config.FORCE_SURFACE * (avgPos - posA).x  << std::endl;
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
                    if (!(nameA == "Particle" && nameB == "Particle") && (nameA == "Particle" || nameB == "Particle")) {
                        for (int k = 0; k < manifold.pointCount; ++k) {
                            b2ManifoldPoint point = manifold.points[k];
                            b2Vec2 pos = point.point;
                            //p.nextTickForce += (p.pos - pos) * Config.FORCE_ADHESION;
                            //    std::cout<<"AWA\n";
                        }
                    }
                }
                //*/
            }
            return;
        }
        /*
        void ParticleGroup::ComputeChunkForces(int start, int end) {
            for (int idx = start; idx < end; ++idx) {
                Particle& p = Particles[idx];
                QueryContext ctx;
                ctx.current = &p;
                ctx.particleGroup = this; b2Vec2 posA = p.pos;
                float influenceRange = p.shape.radius * Config.Impact;
                b2AABB queryAABB;
                queryAABB.lowerBound = posA - b2Vec2{ influenceRange, influenceRange };
                queryAABB.upperBound = posA + b2Vec2{ influenceRange, influenceRange };
                b2DynamicTree_Query(&dynamicTree, queryAABB, B2_DEFAULT_MASK_BITS, QueryCallback, &ctx);
                p.neighborCount = ctx.neighbors.size();
                float radiusA = p.shape.radius / (p.shape.radius / Config.Impact);

                bool havefreezep = false;
                b2Vec2 neighborSum = b2Vec2_zero;
                int attractCount = 0; 

                for (auto* other : ctx.neighbors) {
                    if (b2Body_GetType(other->bodyId) == b2_staticBody) {
                        p.FreezeTime++;
                        havefreezep = true;
                    }
                    if (b2Body_GetType(p.bodyId) == b2_staticBody) {
                        p.FreezeTime = 30;
                        havefreezep = true;
                    }
                    else if (p.FreezeTime >= 30) {
        b2Body_SetType(p.bodyId, b2_staticBody);
                    }
                    b2Vec2 posB = other->pos;
                    float radiusB = other->shape.radius / (p.shape.radius / Config.Impact);
                    b2Vec2 offset = posB - posA;
                    float dst = MathUtils::b2Vec2Length(offset) / (p.shape.radius / Config.Impact);
                    float effectiveRange = (radiusA + radiusB) * Config.Impact;
                    if (dst < effectiveRange) {

                        // ... 在 ComputeChunkForces 中 for(auto* other : ctx.neighbors) 里 ...

                        // 计算当前距离和方向
                        b2Vec2 dpos = other->pos - p.pos;
                        float dist = MathUtils::b2Vec2Length(dpos);
                        if (dist <= 0.0f) continue;
                        b2Vec2 dir = { dpos.x / dist, dpos.y / dist };


                        dst = dst - radiusA;
                        b2Vec2 forceDir = MathUtils::b2Vec2Normalized(offset);
                        float distanceForceMag = GetForce(dst / ctx.density, effectiveRange);
                        b2Vec2 repulsionForce = (-Config.FORCE_MULTIPLIER) * forceDir * distanceForceMag * effectiveRange;
                        b2Vec2 velA = p.LinearVelocity;
                        b2Vec2 velB = other->LinearVelocity;
                        b2Vec2 momentumForce = (velA - velB) * ((effectiveRange - dst) / effectiveRange) * Config.MomentumCoefficient;
                        float d0 = p.shape.radius * Config.Impact;
                        b2Vec2 springForce = b2Vec2_zero;

                        if (MathUtils::b2Vec2Length(offset) > d0) {
                            neighborSum += posB;
                            attractCount++;
                        }

                        // —— 在这里计算 viscForce ——
                        // 1) 先算一个距离衰减权重 W （0..1）
                        float range = (radiusA + radiusB) * Config.Impact;
                        float W = (range - dist) / range;
                        W = std::max(W, 0.0f);

                        // 2) 线性粘滞力：越靠近（W 越大），阻力越明显
                        b2Vec2 velDiff = p.LinearVelocity - other->LinearVelocity;
                        b2Vec2 viscForce = -Config.PLASTICITY_RATE * W * velDiff;

                        // （这里保留你原来的排斥力/动量力、粘性力等合成 totalForce）
                        b2Vec2 totalForce = repulsionForce + momentumForce + springForce + viscForce;

                        // 最后施加
                        std::lock_guard<std::mutex> lk(mutex);
                        other->nextTickLinearImpulse += totalForce;
                        p.nextTickLinearImpulse -= totalForce;

                    }
                }
                if (!havefreezep) {
                    p.FreezeTime = std::max(p.FreezeTime - 1, 0);
                }
                if (attractCount > 0) {
                    b2Vec2 avgPos = { neighborSum.x / attractCount, neighborSum.y / attractCount };
                    p.nextTickLinearImpulse += Config.FORCE_SURFACE * (avgPos - posA);
                    //std::cout << Config.FORCE_SURFACE * (avgPos - posA).x  << std::endl;
                }
            }
            return;
        }*/
        void ParticleGroup::ComputeParticleForces() {
            const int n = Particles.size();
            if (n == 0) return;

            const int threadCount = std::min(std::max(n / 500, 1), 8);
            int chunkSize = n / threadCount;
            std::vector<std::thread> threads;

            for (int ti = 0; ti < threadCount; ++ti) {
                int start = ti * chunkSize;
                int end = std::min(start + chunkSize, n);
                threads.emplace_back(&ParticleGroup::ComputeChunkForces, this, start, end);
            }

            for (auto& thread : threads) {
                thread.join();
            }
        }


        /*
    void ParticleGroup::ComputeParticleForces() {
        const int n = Particles.size();
        if (n == 0) return;

        const int threadCount = std::min(std::max(n / 500, 1), 8);
        int chunkSize = std::max(n / threadCount, 500);
        std::atomic<int> tasksRemaining(threadCount);
        std::mutex finishMtx;
        std::condition_variable finishCv;

        for (int ti = 0; ti < threadCount; ++ti) {
            int start = ti * chunkSize;
            int end = std::min(start + chunkSize, n);
            if (start >= end) {
                tasksRemaining--;
                continue;
            }
            pool->enqueue([this, start, end, &tasksRemaining, &finishCv]() {
                for (int idx = start; idx < end; ++idx) {
                    Particle& p = Particles[idx];
                    QueryContext ctx;
                    ctx.current = &p;
                    ctx.particleGroup = this; b2Vec2 posA = p.pos;
                    float influenceRange = p.shape.radius * Config.Impact;
                    b2AABB queryAABB;
                    queryAABB.lowerBound = posA - b2Vec2{ influenceRange, influenceRange };
                    queryAABB.upperBound = posA + b2Vec2{ influenceRange, influenceRange };
                    b2DynamicTree_Query(&dynamicTree, queryAABB, B2_DEFAULT_MASK_BITS, QueryCallback, &ctx);
					p.neighborCount = ctx.neighbors.size();
                    float radiusA = p.shape.radius / (p.shape.radius / Config.Impact);
                    for (auto* other : ctx.neighbors) {
                        b2Vec2 posB = other->pos;
                        float radiusB = other->shape.radius / (p.shape.radius / Config.Impact);
                        b2Vec2 offset = posB - posA;
                        float dst = MathUtils::b2Vec2Length(offset) / (p.shape.radius / Config.Impact);
                        float effectiveRange = (radiusA + radiusB) * Config.Impact;
                        if (dst < effectiveRange) {
                            dst = dst - radiusA;
                            b2Vec2 forceDir = MathUtils::b2Vec2Normalized(offset);
                            float distanceForceMag = GetForce(dst / ctx.density, effectiveRange);
                            b2Vec2 repulsionForce = (-Config.FORCE_MULTIPLIER) * forceDir * distanceForceMag * effectiveRange;
                            b2Vec2 velA = p.LinearVelocity;
                            b2Vec2 velB = other->LinearVelocity;
                            b2Vec2 momentumForce = (velA - velB) * ((effectiveRange - dst) / effectiveRange) * Config.MomentumCoefficient;
                            float d0 = radiusA + radiusB;
                            b2Vec2 springForce = b2Vec2_zero;
                            if (dst + radiusA > d0) {
                                float kSurface = Config.FORCE_SURFACE * p.shape.radius;
                                springForce = -kSurface * (dst - d0 + radiusA) * forceDir;
                                other->nextTickForce += springForce;
                                p.nextTickForce -= springForce;
                            }
                            b2Vec2 totalForce = repulsionForce + momentumForce + springForce;
                            totalForce *= (p.shape.radius / 2.0f);
                            other->nextTickLinearImpulse += totalForce;
                            p.nextTickLinearImpulse -= totalForce;
                        }
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
                        if (!(nameA == "Particle" && nameB == "Particle") && (nameA == "Particle" || nameB == "Particle")) {
                            for (int k = 0; k < manifold.pointCount; ++k) {
                                b2ManifoldPoint point = manifold.points[k];
                                b2Vec2 pos = point.point;
                                //p.nextTickForce += (p.pos - pos) * Config.FORCE_ADHESION;
                                //    std::cout<<"AWA\n";
                            }
                        }
                    }
                    //*/
                    /*}
                // 通知完成
                if (--tasksRemaining == 0) {
                    finishCv.notify_one();
                }
                });
        }

        std::unique_lock lk(finishMtx);
        finishCv.wait(lk, [&] { return tasksRemaining.load() == 0; });
    }
    */
}