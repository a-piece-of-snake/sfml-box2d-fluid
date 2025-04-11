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

    void ParticleGroup::CreateParticle(GameObjects::World world,  float radius, float x, float y, float density, float friction, float restitution) {
        b2BodyDef bodyDef = b2DefaultBodyDef();
        bodyDef.position = { x, y };
        bodyDef.type = b2_dynamicBody;
        bodyDef.linearDamping = 0.01f;
        bodyDef.angularDamping = 0.01f;
        bodyDef.sleepThreshold = 0.05f;
        bodyDef.fixedRotation = true;
        bodyDef.isBullet = false;
        bodyDef.name = "Particle";
        bodyDef.sleepThreshold = radius / 5.f;
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

        const float queryRange = radius * Config.Impact;
        b2AABB aabb;
        aabb.lowerBound = b2Vec2{ x - queryRange, y - queryRange };
        aabb.upperBound = b2Vec2{ x + queryRange, y + queryRange };

        GameObjects::Particle p;
        p.bodyId = bodyId;
        p.shape = circle;
        p.shapeId = shapeId;
        int index = Particles.size();
        p.proxyId = b2DynamicTree_CreateProxy(&dynamicTree, aabb, B2_DEFAULT_CATEGORY_BITS, index);
        Particles.push_back(p);
        proxyMap[p.proxyId] = &Particles.back();
    }
        void ParticleGroup::DestroyParticle(GameObjects::World world, GameObjects::Particle* particle) {
            b2DestroyBody(particle->bodyId);
            b2DynamicTree_DestroyProxy(&dynamicTree, particle->proxyId);
            proxyMap.erase(particle->proxyId);
            auto it = std::find(Particles.begin(), Particles.end(), *particle);
            if (it != Particles.end()) {
                particle = &(*Particles.erase(it));
            }
            else {
                std::cerr << "Warning: Particle not found in Particles container." << std::endl;
            }
        }

        bool GameObjects::ParticleGroup::QueryCallback(int proxyId, int userData, void* context) {
            // 将 context 转换为 ParticleGroup 指针
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

            ctx->neighbors.push_back(other);
            return true;
        }


    float ParticleGroup::GetForce(float dst, float radius) { // thank you Sebastian Lague
        if (dst >= radius)
            return 0;
        if (dst < 0.01f)
            dst = 0.01f;
        float volume = (B2_PI * std::pow(radius, 4)) / 6;
        return (radius - dst) * (radius - dst) / volume;
    }
    void ParticleGroup::ComputeParticleForces() {
        //石山代码qwq
        for (auto p : Particles) {
            GameObjects::QueryContext ctx;
            ctx.current = &p;
            ctx.particleGroup = this;
            b2Vec2 posA = p.pos;//b2Body_GetPosition(p.bodyId);
            float influenceRange = p.shape.radius * Config.Impact;
            b2AABB queryAABB;
            queryAABB.lowerBound = posA - b2Vec2{ influenceRange / 2.f, influenceRange / 2.f };
            queryAABB.upperBound = posA + b2Vec2{ influenceRange / 2.f, influenceRange / 2.f };
            b2DynamicTree_Query(&dynamicTree, queryAABB, B2_DEFAULT_MASK_BITS, QueryCallback, &ctx);//here
            float radiusA = p.shape.radius / (p.shape.radius / Config.Impact);
            //p.CloseParticles.clear();
            for (GameObjects::Particle* other : ctx.neighbors) {
                b2Vec2 posB = other->pos;// b2Body_GetPosition(other->bodyId);
                float radiusB = other->shape.radius / (p.shape.radius / Config.Impact);
                b2Vec2 offset = posB - posA;
                float dst = MathUtils::b2Vec2Length(offset) / (p.shape.radius / Config.Impact);
                float effectiveRange = (radiusA + radiusB) * Config.Impact;
                if (dst < effectiveRange) {
                    //p.CloseParticles.push_back(&other);
                    float density = ctx.neighbors.size();
                    float densityForce = density / 10.f;
                    float densityForce2 = densityForce;
                    if (densityForce < 1.f) densityForce = 1.f;
                    else if (densityForce > 10.f) densityForce = 10.f;
                    b2Vec2 forceDir = MathUtils::b2Vec2Normalized(offset);
                    float distanceForceMag = GetForce(dst / densityForce, effectiveRange);
                    if (densityForce > 10.f) distanceForceMag *= densityForce2;
                    b2Vec2 repulsionForce = (-Config.FORCE_MULTIPLIER) * forceDir * distanceForceMag * effectiveRange;// *densityForce;
                    b2Vec2 velA = p.LinearVelocity;// b2Body_GetLinearVelocity(p.bodyId);
                    b2Vec2 velB = other->LinearVelocity;// b2Body_GetLinearVelocity(other->bodyId);
                    b2Vec2 momentumForce = (velA - velB) * ((effectiveRange - dst) / effectiveRange) * Config.MomentumCoefficient;
                    float d0 = radiusA + radiusB;
                    b2Vec2 springForce = b2Vec2_zero;
                    if (dst > d0) {
                        float kSurface = Config.FORCE_SURFACE * p.shape.radius;
                        springForce = -kSurface * (dst - d0) * forceDir;
                        other->nextTickForce += springForce;
                        p.nextTickForce -= springForce;
                    }
                    b2Vec2 totalForce = repulsionForce + momentumForce + springForce;
                    totalForce *= (p.shape.radius / 2.0f);
                    other->nextTickLinearImpulse += totalForce;
                    p.nextTickLinearImpulse -= totalForce;
                    //b2Body_ApplyLinearImpulseToCenter(other->bodyId, totalForce, true);
                    //b2Body_ApplyLinearImpulseToCenter(p.bodyId, -totalForce, true);
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

                        b2WeldJointDef jointDef = b2DefaultWeldJointDef();
                        jointDef.bodyIdA = bodyIdA;
                        jointDef.bodyIdB = bodyIdB;
                        jointDef.localAnchorA = b2Body_GetLocalPoint(jointDef.bodyIdA, pos);
                        jointDef.localAnchorB = b2Body_GetLocalPoint(jointDef.bodyIdB, pos);
                        jointDef.angularHertz = fluid.Config.FORCE_ADHESION;
                        jointDef.angularDampingRatio = fluid.Config.FORCE_ADHESION;
                        jointDef.linearHertz = fluid.Config.FORCE_ADHESION;
                        jointDef.linearDampingRatio = fluid.Config.FORCE_ADHESION;
                        jointDef.collideConnected = true;
                        p.AdhesionJoint.push_back(b2CreateWeldJoint(world.worldId, &jointDef));
                    }
                }
                for (int i = 0; i < p.AdhesionJoint.size(); ++i)
                {
                    if (B2_IS_NULL(p.AdhesionJoint[i]))
                    {
                        continue;
                    }

                    b2Vec2 force = b2Joint_GetConstraintForce(p.AdhesionJoint[i]);
                    b2Vec2 anchorA = b2Joint_GetLocalAnchorA(p.AdhesionJoint[i]);// +b2Body_GetPosition(b2Joint_GetBodyA(p.AdhesionJoint[i]));
                    b2Vec2 anchorB = b2Joint_GetLocalAnchorB(p.AdhesionJoint[i]);// +b2Body_GetPosition(b2Joint_GetBodyB(p.AdhesionJoint[i]));
                    float distance = MathUtils::b2Vec2Length(anchorB - anchorA);
                    std::cout << distance << std::endl;
                    if (distance > fluid.Config.FORCE_ADHESION)
                    {
                        //b2DestroyJoint(p.AdhesionJoint[i]);
                        //p.AdhesionJoint.erase(p.AdhesionJoint[i]);
                        //p.AdhesionJoint[i] = b2_nullJointId;
                    }
                }
            }
            */
        }
    }
}