#include <iostream>
#include <vector>
#include <cstdlib>
#include <cmath>
#include <box2d/box2d.h>
#include <box2d/collision.h>
#include <box2d/math_functions.h>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include "renderBox2d.h"
#include "GameObjects.h"
#include "MathUtils.h"
#include <random>
#include <iomanip>
#include <functional>
#include <thread>
#include <vector>
#include <numeric>
int width = 2400;
int height = 1350;
int squareCount = 0;


sf::RenderWindow window(sf::VideoMode({ 2400, 1350 }), "AWA", sf::Style::Default);
sf::Vector2i lastmousePos;
bool Drag = false;
struct World
{
    b2WorldId worldId;

}world;
struct SelectionBox {
    bool isSelecting = false;
    sf::Vector2f startPos;
    sf::Vector2f currentPos;
} selection;

bool PauseWorld = false;
std::vector<std::pair<b2BodyId, b2Polygon>> squares;
//std::vector<GameObjects::Particle> particles;
GameObjects::ParticleGroup fluid;
b2DynamicTree dynamicTree;
std::unordered_map<int, GameObjects::Particle*> proxyMap;

b2Vec2 toWorldPosition(float screenX, float screenY) {
    return b2Vec2{
        (height / 2.0f - screenY + std::abs(width - height) / 2),
        -(screenX - width / 2.0f + std::abs(width - height) / 2)
    };
}

float camX = 0.f, camY = 0.f;

float GetForce(float dst, float radius) { // thank you Sebastian Lague
    if (dst >= radius)
        return 0;
    if (dst < 0.01f)
        dst = 0.01f;
    float volume = (B2_PI * std::pow(radius, 4)) / 6;
    return (radius - dst) * (radius - dst) / volume;
}

void CreateParticle(float radius, float x, float y, float density, float friction, float restitution) {
    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.position = { x, y };
    bodyDef.type = b2_dynamicBody;
    bodyDef.linearDamping = 0.01f;
    bodyDef.angularDamping = 0.01f;
    bodyDef.sleepThreshold = 0.05f;
    bodyDef.fixedRotation = true;
    bodyDef.isBullet = false;
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
    b2CreateCircleShape(bodyId, &shapeDef, &circle);

    const float queryRange = radius * fluid.Config.Impact;
    b2AABB aabb;
    aabb.lowerBound = b2Vec2{ x - queryRange, y - queryRange };
    aabb.upperBound = b2Vec2{ x + queryRange, y + queryRange };

    GameObjects::Particle p;
    p.bodyId = bodyId;
    p.shape = circle;
    int index = fluid.Particles.size();
    p.proxyId = b2DynamicTree_CreateProxy(&dynamicTree, aabb, B2_DEFAULT_CATEGORY_BITS, index);
    fluid.Particles.push_back(p);
    proxyMap[p.proxyId] = &fluid.Particles.back();
}

void createSquare(float x, float y) {
    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = b2_dynamicBody;
    bodyDef.position = b2Vec2{ x, y };
    b2BodyId bodyId = b2CreateBody(world.worldId, &bodyDef);

    b2Polygon square = b2MakeBox(10.0f, 10.0f);
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.density = 1.0f;
    shapeDef.friction = 0.3f;
    shapeDef.restitution = 0.1f;
    b2CreatePolygonShape(bodyId, &shapeDef, &square);
    squares.push_back({ bodyId, square });
}

void UpdateDynamicTree() {
    for (auto& p : fluid.Particles) {
        b2Vec2 pos = b2Body_GetPosition(p.bodyId);
        float radius = p.shape.radius;
        const float queryRange = radius * fluid.Config.Impact;
        b2AABB newAABB;
        newAABB.lowerBound = b2Vec2{ pos.x - queryRange, pos.y - queryRange };
        newAABB.upperBound = b2Vec2{ pos.x + queryRange, pos.y + queryRange };
        b2DynamicTree_MoveProxy(&dynamicTree, p.proxyId, newAABB);
    }
}

struct QueryContext {
    GameObjects::Particle* current;
    std::vector<GameObjects::Particle*> neighbors;
};

static bool QueryCallback(int proxyId, int userData, void* context) {
    QueryContext* ctx = static_cast<QueryContext*>(context);
    if (proxyMap.find(proxyId) == proxyMap.end())
        return true;
    int index = userData;
    if (index < 0 || index >= static_cast<int>(fluid.Particles.size()))
        return true;
    GameObjects::Particle* other = &fluid.Particles[index];
    if (other == ctx->current)
        return true;
    ctx->neighbors.push_back(other);
    return true;
}


void ComputeParticleForces() {
    //石山代码qwq
    for (auto& p : fluid.Particles) {

        QueryContext ctx;
        ctx.current = &p;
        b2Vec2 posA = b2Body_GetPosition(p.bodyId);
        float influenceRange = p.shape.radius * fluid.Config.Impact;
        b2AABB queryAABB;
        queryAABB.lowerBound = posA - b2Vec2{ influenceRange / 2, influenceRange / 2 };
        queryAABB.upperBound = posA + b2Vec2{ influenceRange / 2, influenceRange / 2 };
        b2DynamicTree_Query(&dynamicTree, queryAABB, B2_DEFAULT_MASK_BITS, QueryCallback, &ctx);
        float radiusA = p.shape.radius / (p.shape.radius / fluid.Config.Impact);
        //p.CloseParticles.clear();
        for (const GameObjects::Particle* other : ctx.neighbors) {
            b2Vec2 posB = b2Body_GetPosition(other->bodyId);
            float radiusB = other->shape.radius / (p.shape.radius / fluid.Config.Impact);
            b2Vec2 offset = posB - posA;
            float dst = MathUtils::b2Vec2Length(offset) / (p.shape.radius / fluid.Config.Impact);
            float effectiveRange = (radiusA + radiusB) * fluid.Config.Impact;
            if (dst == 0.f) {
                float randomAngle = (std::rand() / (float)RAND_MAX) * 2 * B2_PI;
                b2Vec2 randomForce = b2Vec2{ std::cos(randomAngle), std::sin(randomAngle) } * 0.001f;
                b2Body_ApplyForce(other->bodyId, randomForce, posB, true);
                b2Body_ApplyForce(p.bodyId, -randomForce, posA, true);
                continue;
            }
            else if (dst < effectiveRange) {
                //p.CloseParticles.push_back(&other);
                float density = ctx.neighbors.size();
                float densityForce = density / 15.f;
                float densityForce2 = densityForce;
                if (densityForce < 1.f) densityForce = 1.f;
                else if (densityForce > 8.f) densityForce = 8.f;
                b2Vec2 forceDir = MathUtils::b2Vec2Normalized(offset);
                float distanceForceMag = GetForce(dst / densityForce, effectiveRange);
                if (densityForce > 10.f) distanceForceMag *= densityForce2;
                b2Vec2 repulsionForce = (-fluid.Config.FORCE_MULTIPLIER) * forceDir * distanceForceMag * effectiveRange;// *densityForce;
                float momentumCoefficient = 1.0f;
                b2Vec2 velA = b2Body_GetLinearVelocity(p.bodyId);
                b2Vec2 velB = b2Body_GetLinearVelocity(other->bodyId);
                b2Vec2 momentumForce = (velA - velB) * momentumCoefficient * ((effectiveRange - dst) / effectiveRange);
                float d0 = radiusA + radiusB;
                b2Vec2 springForce = b2Vec2_zero;
                if (dst > d0) {
                    float kSurface = fluid.Config.FORCE_SURFACE * p.shape.radius;
                    springForce = -kSurface * (dst - d0) * forceDir;
                    springForce *= (p.shape.radius / 2.0f);
                    b2Body_ApplyForceToCenter(other->bodyId, springForce, true);
                    b2Body_ApplyForceToCenter(p.bodyId, -springForce, true);
                }
                b2Vec2 totalForce = repulsionForce + momentumForce + springForce;
                totalForce *= (p.shape.radius / 2.0f);
                b2Body_ApplyLinearImpulseToCenter(other->bodyId, totalForce, true);
                b2Body_ApplyLinearImpulseToCenter(p.bodyId, -totalForce, true);
            }
        }

    }
}
int main() {
    window.setFramerateLimit(60);
    dynamicTree = b2DynamicTree_Create();
    int numThreads = std::thread::hardware_concurrency(); 
    if (numThreads == 0) numThreads = 4;
    //else if (numThreads > 4) numThreads = 4;
    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = b2Vec2{ -2.5f, 0.0f };
    worldDef.contactHertz = 15.f;
    worldDef.enableContinuous = false;
    worldDef.workerCount = numThreads;
    world.worldId = b2CreateWorld(&worldDef);

    b2BodyDef groundBodyDef = b2DefaultBodyDef();
    groundBodyDef.type = b2_staticBody;
    groundBodyDef.position.x = 0.0f;
    groundBodyDef.position.y = 100.0f;
    b2BodyId groundId = b2CreateBody(world.worldId, &groundBodyDef);
    b2Polygon groundBox = b2MakeBox(30.0f, 2000.0f);
    b2ShapeDef groundShapeDef = b2DefaultShapeDef();
    groundShapeDef.density = 0.0f;
    b2CreatePolygonShape(groundId, &groundShapeDef, &groundBox);

    b2BodyDef groundBodyDef2 = b2DefaultBodyDef();
    groundBodyDef2.position.x = 75.0f;
    groundBodyDef2.position.y = 0.0f;
    groundBodyDef2.type = b2_staticBody;
    b2BodyId groundId2 = b2CreateBody(world.worldId, &groundBodyDef2);
    b2Polygon groundBox2 = b2MakeBox(500.f, 10.0f);
    b2ShapeDef groundShapeDef2 = b2DefaultShapeDef();
    groundShapeDef2.density = 0.0f;
    b2CreatePolygonShape(groundId2, &groundShapeDef2, &groundBox2);

    b2BodyDef groundBodyDef3 = b2DefaultBodyDef();
    groundBodyDef3.position.x = 300.0f;
    groundBodyDef3.position.y = 500.0f;
    groundBodyDef3.type = b2_staticBody;
    b2BodyId groundId3 = b2CreateBody(world.worldId, &groundBodyDef3);
    b2Polygon groundBox3 = b2MakeBox(200.f, 10.0f);
    b2ShapeDef groundShapeDef3 = b2DefaultShapeDef();
    groundShapeDef3.density = 0.0f;
    b2CreatePolygonShape(groundId3, &groundShapeDef3, &groundBox3);

    b2BodyDef groundBodyDef4 = b2DefaultBodyDef();
    groundBodyDef4.position.x = 75.0f;
    groundBodyDef4.position.y = 500.0f;
    groundBodyDef4.type = b2_staticBody;
    b2BodyId groundId4 = b2CreateBody(world.worldId, &groundBodyDef4);
    b2Polygon groundBox4 = b2MakeBox(500.f, 10.0f);
    b2ShapeDef groundShapeDef4 = b2DefaultShapeDef();
    groundShapeDef4.density = 0.0f;
    b2CreatePolygonShape(groundId4, &groundShapeDef4, &groundBox4);
    //{
    //    GameObjects::ParticleConfig Config;
    //    Config.
    //    fluid.Config = Config;
    //}
    float timeStep = 0.2f;
    int subStepCount = 10;
    renderB2::RenderSettings rendersettings;
    rendersettings.OutlineThickness = 1;
    rendersettings.OutlineColor = sf::Color::Magenta;
    rendersettings.FillColor = sf::Color::White;
    rendersettings.verticecount = 4;

    sf::Texture BackGroundT("Assets\\Textures\\BackGround.png");
    BackGroundT.setRepeated(true);
    sf::RectangleShape BackGround({ 1525.f, 610.f });
    BackGround.move({ 100.f, 100.f });
    BackGround.setOutlineThickness(3);
    BackGround.setOutlineColor(sf::Color(0, 98, 167));
    BackGround.setTexture(&BackGroundT);
    BackGround.setTextureRect(sf::IntRect{ { 0, 0 }, { (int)(BackGround.getSize().x / 1.5), (int)(BackGround.getSize().y / 1.5) } });

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
            if (const auto* mouseReleased = event->getIf<sf::Event::MouseButtonReleased>()) {
                if (mouseReleased->button == sf::Mouse::Button::Left && selection.isSelecting) {
                    sf::Vector2f worldStart = window.mapPixelToCoords(sf::Vector2i(selection.startPos));
                    sf::Vector2f worldEnd = window.mapPixelToCoords(sf::Vector2i(selection.currentPos));

                    b2Vec2 start = toWorldPosition(worldStart.x, worldStart.y);
                    b2Vec2 end = toWorldPosition(worldEnd.x, worldEnd.y);

                    float minX = std::min(start.x, end.x);
                    float maxX = std::max(start.x, end.x);
                    float minY = std::min(start.y, end.y);
                    float maxY = std::max(start.y, end.y);
                    float Xlen = std::abs(maxX - minX);
                    float Ylen = std::abs(maxY - minY);
                    const float gridSize = 200;
                    float stepX = (maxX - minX) / (Xlen * gridSize / window.getSize().x);
                    float stepY = (maxY - minY) / (Ylen * gridSize / window.getSize().y);
                    std::cout << "selection : x:" << (Xlen * gridSize / window.getSize().x) << "y:" << (Ylen * gridSize / window.getSize().y) << std::endl;
                    for (int i = 0; i < (Xlen * gridSize / window.getSize().x); ++i) {
                        for (int j = 0; j < (Ylen * gridSize / window.getSize().y); ++j) {
                            float x = minX + i * stepX + stepX / 2;
                            float y = minY + j * stepY + stepY / 2;
                            CreateParticle(3, x - camX, y + camY, 2.5f, 0.f, 0.25f);
                            squareCount++;
                        }
                    }
                    std::cout << squareCount << std::endl;
                    selection.isSelecting = false;
                }
            }
            if (const auto* mouseMoved = event->getIf<sf::Event::MouseMoved>()) {
                //if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Middle)) {
                //    camX -= lastmousePos.x - mouseMoved->position.x;
                //    camY -= lastmousePos.y - mouseMoved->position.y;
                //}
                if (selection.isSelecting) {
                    selection.currentPos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
                }
            }
            if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
            {
                if (keyPressed->scancode == sf::Keyboard::Scan::Space)
                {
                    PauseWorld = !PauseWorld;
                }
            }
        }
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        b2Vec2 worldPos = toWorldPosition(mousePos.x, mousePos.y);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num2)) {
            squareCount++;
            CreateParticle(3, worldPos.x - camX, worldPos.y + camY, 2.5f, 0.f, 0.25f);
            std::cout << squareCount << std::endl;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num1)) {
            createSquare(worldPos.x - camX, worldPos.y + camY);
        }
        if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Right)) {
            for (auto& [bodyId, shape] : squares)
                b2DestroyBody(bodyId);
            for (auto& p : fluid.Particles)
                b2DestroyBody(p.bodyId);
            squares.clear();
            fluid.Particles.clear();
            squareCount = 0;
            std::cout << "All deleted." << std::endl;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift)) {
            selection.isSelecting = true;
            selection.startPos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
            selection.currentPos = selection.startPos;
        }

        b2ExplosionDef Dragdef = b2DefaultExplosionDef();

        if (!PauseWorld) {
            UpdateDynamicTree();
            for (auto it = fluid.Particles.begin(); it != fluid.Particles.end(); ) {
                it->pos = b2Body_GetPosition(it->bodyId);
                it->LinearVelocity = b2Body_GetLinearVelocity(it->bodyId);
                /*if (it->LinearVelocity.x < 0.05f && it->LinearVelocity.y < 0.05f) {
                    b2Body_SetLinearVelocity(it->bodyId, { 0.f, 0.f });
                }*/
                if (it->age >= it->life && it->life >= 0) {
                    b2DestroyBody(it->bodyId);
                    b2DynamicTree_DestroyProxy(&dynamicTree, it->proxyId);
                    proxyMap.erase(it->proxyId);
                    it = fluid.Particles.erase(it);
                }
                else {
                    it->age++;
                    ++it;
                }
            }
            /*
            // 分割粒子数据
            int particlesPerThread = particles.size() / numThreads;
            std::vector<std::thread> threads;

            for (int i = 0; i < numThreads; ++i) {
                int start = i * particlesPerThread;
                int end = (i == numThreads - 1) ? particles.size() : (i + 1) * particlesPerThread;
                std::vector<GameObjects::Particle> particleSubset;
                for (int j = start; j < end; ++j) {
                    particleSubset.push_back(particles[j]);
                }
                //std::cout << "awaw"<<std::endl;
                threads.emplace_back(ComputeParticleForcesRange, std::ref(particleSubset), std::ref(particles));
                //std::cout << "wawawa" << std::endl;
            }
            // 等待所有线程完成
            for (auto& thread : threads) {
                thread.join();
            }
            //ComputeParticleForces();
            */
            ComputeParticleForces();
            b2World_Step(world.worldId, timeStep, subStepCount);
            if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
                Drag = true;
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                Dragdef.position = b2Vec2{ worldPos.x - camX, worldPos.y + camY };
                Dragdef.radius = 70.0f;
                Dragdef.falloff = 5.f;
                Dragdef.impulsePerLength = -50.0f;
                b2World_Explode(world.worldId, &Dragdef);
            }
            if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Middle)) {
                Drag = true;
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                Dragdef.position = b2Vec2{ worldPos.x - camX, worldPos.y + camY };
                Dragdef.radius = 25.0f;
                Dragdef.falloff = 0.f;
                Dragdef.impulsePerLength = 100.0f;
                b2World_Explode(world.worldId, &Dragdef);
            }
            if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Middle) && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl)) {
                b2BodyDef bodyDef = b2DefaultBodyDef();
                bodyDef.type = b2_staticBody;
                bodyDef.position = toWorldPosition(mousePos.x, mousePos.y);
                b2BodyId bodyId = b2CreateBody(world.worldId, &bodyDef);

                b2Polygon square = b2MakeBox(10.0f, 10.0f);
                b2ShapeDef shapeDef = b2DefaultShapeDef();
                b2CreatePolygonShape(bodyId, &shapeDef, &square);
                squares.push_back({ bodyId, square });
            }
        }
        window.clear(sf::Color::Black);
        window.draw(BackGround);

        renderB2::ScreenSettings screensettings;
        screensettings.camX = camX;
        screensettings.camY = camY;
        screensettings.width = width;
        screensettings.height = height;
        renderB2::renderb2Polygon(&window, rendersettings, groundBox, groundId, screensettings);
        renderB2::renderb2Polygon(&window, rendersettings, groundBox2, groundId2, screensettings);
        renderB2::renderb2Polygon(&window, rendersettings, groundBox3, groundId3, screensettings);
        renderB2::renderb2Polygon(&window, rendersettings, groundBox4, groundId4, screensettings);


        for (auto& [bodyId, shape] : squares) {
            renderB2::renderb2Polygon(&window, rendersettings, shape, bodyId, screensettings);
        }

        rendersettings.verticecount = 3;
        for (auto& p : fluid.Particles) {
            renderB2::rendersimpleparticle(&window, rendersettings, p.shape, p.bodyId, screensettings);
        }
        rendersettings.verticecount = 4;

        if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) || sf::Mouse::isButtonPressed(sf::Mouse::Button::Middle)) {
            sf::CircleShape shape(Dragdef.radius, 32);
            shape.setOutlineThickness(2);
            shape.setOutlineColor(sf::Color::Blue);
            shape.setFillColor(sf::Color::Transparent);
            shape.setOrigin({ Dragdef.radius, Dragdef.radius });
            shape.setPosition(sf::Vector2f(mousePos.x, mousePos.y));
            window.draw(shape);
        }

        if (selection.isSelecting) {
            sf::RectangleShape selectionBox;
            sf::Vector2f size = selection.currentPos - selection.startPos;
            selectionBox.setSize(sf::Vector2f(std::abs(size.x), std::abs(size.y)));
            selectionBox.setPosition({ std::min(selection.startPos.x, selection.currentPos.x),
                std::min(selection.startPos.y, selection.currentPos.y) });
            selectionBox.setFillColor(sf::Color(100, 100, 255, 50));
            selectionBox.setOutlineColor(sf::Color::Blue);
            selectionBox.setOutlineThickness(2);
            window.draw(selectionBox);
        }
        window.display();
        lastmousePos = mousePos;
    }

    b2DynamicTree_Destroy(&dynamicTree);
    b2DestroyWorld(world.worldId);
    return 0;
}
/*
void ComputeParticleForcesRange(std::vector<GameObjects::Particle>& particleSubset, std::vector<GameObjects::Particle>& allParticles) {
    const float FORCE_MULTIPLIER = -500.0f;
    for (auto& p : particleSubset) {
        QueryContext ctx;
        ctx.current = &p;
        //3月15号的蛇说 这里不要用 box2d 的方法 会导致 world 数量超过；
        //
        ///*
        b2Vec2 posA = p.pos;//here
        float influenceRange = p.shape.radius * 3.f;
        b2AABB queryAABB;
        queryAABB.lowerBound = posA - b2Vec2{ influenceRange / 2, influenceRange / 2 };
        queryAABB.upperBound = posA + b2Vec2{ influenceRange / 2, influenceRange / 2 };
        b2DynamicTree_Query(&dynamicTree, queryAABB, B2_DEFAULT_MASK_BITS, QueryCallback, &ctx);
        float radiusA = p.shape.radius / (p.shape.radius / 3.f);
         for (GameObjects::Particle* other : ctx.neighbors) {
            b2Vec2 posB = other->pos;// b2Body_GetPosition(other->bodyId);
            float radiusB = other->shape.radius / (p.shape.radius / 3.f);
            b2Vec2 offset = posB - posA;
            float dst = MathUtils::b2Vec2Length(offset) / (p.shape.radius / 3.f);
            float effectiveRange = (radiusA + radiusB) * 3.f;

            if (dst <= 0.01f) {
                float randomAngle = (std::rand() / (float)RAND_MAX) * 2 * B2_PI;
                b2Vec2 randomForce = b2Vec2{ std::cos(randomAngle), std::sin(randomAngle) } * 0.001f;
                other->nextTickForce += randomForce;
                p.nextTickForce += -randomForce;
                //GameObjects::addForceToParticle(randomForce, other);
                //GameObjects::addForceToParticle(-randomForce, &p);
                //b2Body_ApplyForce(other->bodyId, randomForce, posB, true);
                //b2Body_ApplyForce(p.bodyId, -randomForce, posA, true);
                continue;
            }
            else if (dst < effectiveRange) {
                //p.CloseParticles.push_back(&other);

                float density = ctx.neighbors.size();
                float densityForce = density / 50.f;
                if (densityForce < 1.f) densityForce = 1.f;
                else if (densityForce > 2.f) densityForce = 2.f;
                b2Vec2 forceDir = MathUtils::b2Vec2Normalized(offset);
                float distanceForceMag = GetForce(dst / densityForce, effectiveRange);
                float repulsionMultiplier = 4.f;
                b2Vec2 repulsionForce = (-FORCE_MULTIPLIER) * forceDir * distanceForceMag * repulsionMultiplier * 5 * effectiveRange;// *densityForce;
                float momentumCoefficient = 2.5f;
                b2Vec2 velA = p.LinearVelocity;// b2Body_GetLinearVelocity(p.bodyId);
                b2Vec2 velB = other->LinearVelocity;// b2Body_GetLinearVelocity(other->bodyId);
                b2Vec2 momentumForce = (velA - velB) * momentumCoefficient * ((effectiveRange - dst) / effectiveRange);
                float d0 = radiusA + radiusB;
                b2Vec2 springForce = b2Vec2_zero;
                if (dst > d0) {
                    float kSurface = 2.f * p.shape.radius;
                    springForce = -kSurface * (dst - d0) * forceDir;
                }
                b2Vec2 totalForce = repulsionForce + momentumForce + springForce;
                totalForce *= (p.shape.radius / 3.f);
                //b2Body_ApplyForce(other->bodyId, totalForce, posB, true);
                //b2Body_ApplyForce(p.bodyId, -totalForce, posA, true);
                //GameObjects::addForceToParticle(totalForce, other);
                //GameObjects::addForceToParticle(-totalForce, &p);
                other->nextTickForce += totalForce;
                p.nextTickForce += -totalForce;


            }
        }
    }
}*/