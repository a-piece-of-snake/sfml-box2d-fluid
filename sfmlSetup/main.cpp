
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
#include <SFML/Audio.hpp>
#include "renderBox2d.h"
#include "GameObjects.h"
#include "MathUtils.h"
#include <random>
#include <iomanip>
#include <functional>
#include <thread>
#include <vector>
#include <numeric>
#include <sstream> 

int width = 2400;
int height = 1350;
int particleCount = 0;
sf::RenderWindow window(sf::VideoMode({ 2400, 1350 }), "AWA", sf::Style::Default);
sf::View worldView(sf::FloatRect{ {0.f, 0.f}, {static_cast<float>(width), static_cast<float>(height)} });
sf::View uiView = window.getDefaultView();
sf::Vector2i mousePos;
b2Vec2 worldPos;

sf::Clock timeclock;
int tickCount = 0;
sf::Clock tpsClock;
float tps = 0.0f;
sf::Time elapsed;
float elapsedTime;

sf::Vector2i lastmousePos;
bool Drag = false;
GameObjects::World world;
struct SelectionBox {
    bool isSelecting = false;
    sf::Vector2f startPos;
    sf::Vector2f currentPos;
} selection;
bool PauseWorld = false;
std::vector<std::pair<b2BodyId, b2Polygon>> squares;
//std::vector<GameObjects::Particle> particles;
GameObjects::ParticleGroup fluid;

/*
box2d

↑x
|
|
•——→—————————|
|     y                                    |
|                                           |
|                                           |
|                                           |
|————————————|

sfml
      x
•——→—————————|
|                                           |
|                                           |
↓y                                      |
|                                           |
|————————————|
*/
float camX = 0.f, camY = 0.f;


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

/*
        b2BodyId bodyIdA = b2Shape_GetBody(event.shapeIdA);
        b2BodyId bodyIdB = b2Shape_GetBody(event.shapeIdB);
        std::string nameA = b2Body_GetName(bodyIdA);
        std::string nameB = b2Body_GetName(bodyIdB);
        if (!(nameA == "Particle" && nameB == "Particle") && (nameA == "Particle" || nameB == "Particle"))
        {
            b2BodyId bodyId;
            b2BodyId OtherbodyId;
            if (nameA == "Particle")
                bodyId = bodyIdA, OtherbodyId = bodyIdB;
            else
                bodyId = bodyIdB, OtherbodyId = bodyIdA;

            GameObjects::Particle* particle = nullptr;
            if (proxyMap.find(bodyId.index1) != proxyMap.end()) {
                particle = proxyMap[bodyId.index1];
            }
            if (particle != nullptr) {
                particle->isAdhesion = false;
                particle->adhesionForce = b2Vec2_zero;
            }
        }*/
void RenderUi() {
    {
        sf::ConvexShape ticktime = renderB2::getRectangleMinusCorners(330.f, 115.f, 11.5f);
        ticktime.setFillColor(sf::Color::Transparent);
        ticktime.setOutlineThickness(3.f);
        renderB2::MyUIObject* root = new renderB2::MyUIObject(&ticktime, { 0, 0 }, 0);
        sf::Text awa(renderB2::getDefaultFontAddress());
        awa.setString("Tick Time\n  " + std::to_string((int)elapsedTime) + " ms");
        awa.setStyle(sf::Text::Bold);
        awa.setCharacterSize(30.f);
        if (elapsedTime >= 100) {
            ticktime.setOutlineColor(renderB2::DefaultColors::WarningOutline);
            awa.setFillColor(renderB2::DefaultColors::WarningText);
        }
        else {
            ticktime.setOutlineColor(sf::Color::White);
            awa.setFillColor(sf::Color::White);
        }
        awa.setOrigin({ awa.getLocalBounds().size.x / 2, awa.getLocalBounds().size.y / 2 });
        renderB2::MyUIObject* child = new renderB2::MyUIObject(&awa, sf::Vector2f(50.f, 0.f));
        child->localPosition = { ticktime.getLocalBounds().size.x / 2, ticktime.getLocalBounds().size.y / 2 };
        root->addChild(child);
        root->localPosition = { 8.f, 25.f };
        root->draw(&window);
    }
    {
        if (tpsClock.getElapsedTime().asSeconds() >= 1.0f) {
            tps = tickCount / tpsClock.getElapsedTime().asSeconds();
            tickCount = 0;
            tpsClock.restart();
        }
        sf::ConvexShape tpsshape = renderB2::getRectangleMinusCorners(330.f, 115.f, 11.5f);
        tpsshape.setFillColor(sf::Color::Transparent);
        tpsshape.setOutlineThickness(3.f);
        renderB2::MyUIObject* root = new renderB2::MyUIObject(&tpsshape, { 0, 0 }, 0);
        sf::Text awa(renderB2::getDefaultFontAddress());
        awa.setString("TPS\n" + std::to_string((int)tps));
        awa.setStyle(sf::Text::Bold);
        awa.setCharacterSize(30.f);
        if (tps <= 10) {
            tpsshape.setOutlineColor(renderB2::DefaultColors::WarningOutline);
            awa.setFillColor(renderB2::DefaultColors::WarningText);
        }
        else {
            tpsshape.setOutlineColor(sf::Color::White);
            awa.setFillColor(sf::Color::White);
        }
        awa.setOrigin({ awa.getLocalBounds().size.x / 2, awa.getLocalBounds().size.y / 2 });
        renderB2::MyUIObject* child = new renderB2::MyUIObject(&awa, sf::Vector2f(50.f, 0.f));
        child->localPosition = { tpsshape.getLocalBounds().size.x / 2, tpsshape.getLocalBounds().size.y / 2 };
        root->addChild(child);
        root->localPosition = { 8.f, 151.f };
        root->draw(&window);
    }
    {
        if (tpsClock.getElapsedTime().asSeconds() >= 1.0f) {
            tps = tickCount / tpsClock.getElapsedTime().asSeconds();
            tickCount = 0;
            tpsClock.restart();
        }
        sf::ConvexShape particlecount = renderB2::getRectangleMinusCorners(330.f, 115.f, 11.5f);
        particlecount.setFillColor(sf::Color::Transparent);
        particlecount.setOutlineColor(sf::Color::White);
        particlecount.setOutlineThickness(3.f);
        renderB2::MyUIObject* root = new renderB2::MyUIObject(&particlecount, { 0, 0 }, 0);
        sf::Text awa(renderB2::getDefaultFontAddress());
        awa.setString("Particle Count\n     " + std::to_string(particleCount));
        awa.setStyle(sf::Text::Bold);
        awa.setCharacterSize(30.f);
        awa.setFillColor(sf::Color::White);
        awa.setOrigin({ awa.getLocalBounds().size.x / 2, awa.getLocalBounds().size.y / 2 });
        renderB2::MyUIObject* child = new renderB2::MyUIObject(&awa, sf::Vector2f(50.f, 0.f));
        child->localPosition = { particlecount.getLocalBounds().size.x  / 2, particlecount.getLocalBounds().size.y / 2 };
        root->addChild(child);
        root->localPosition = { 8.f, 277.f };
        root->draw(&window);
    }
    {
        sf::RectangleShape tutorial;
        tutorial.setSize({ 330.f, 330.f });
        tutorial.setFillColor(renderB2::DefaultColors::TextBox);
        tutorial.setOutlineThickness(3.f);
        tutorial.setOutlineColor(sf::Color::White);
        renderB2::MyUIObject* root = new renderB2::MyUIObject(&tutorial, { 0, 0 }, 0);
        sf::Text awa(renderB2::getDefaultFontAddress());
        awa.setString("LShift: Drag-select\narea to create\nparticles\nNum1: Box\nNum2: Particle\nRMB: Clear\nSpace: Pause");
        awa.setStyle(sf::Text::Bold);
        awa.setCharacterSize(30.f);
        awa.setFillColor(sf::Color::White);
        awa.setOrigin( { awa.getLocalBounds().size.x / 2, awa.getLocalBounds().size.y / 2 } );
        renderB2::MyUIObject* child = new renderB2::MyUIObject(&awa, sf::Vector2f(50.f, 0.f));
        child->localPosition = { tutorial.getLocalBounds().size.x / 2, tutorial.getLocalBounds().size.y / 2 };
        root->addChild(child);
        root->localPosition = { 8.f, 403 };
        root->draw(&window);
    }

}
int main() {
    window.setFramerateLimit(60);
    sf::View view(sf::FloatRect({ 0.f, 0.f }, { (float)width, (float)height }));
    window.setView(view);
    int numThreads = std::thread::hardware_concurrency();
    if (numThreads == 0) numThreads = 4;
    //else if (numThreads > 4) numThreads = 4;
    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = b2Vec2{ -2.5f, 0.0f };
    //worldDef.gravity = b2Vec2{ 0.0f, 0.0f };
    worldDef.contactHertz = 15.f;
    worldDef.enableContinuous = false;
    worldDef.workerCount = numThreads;
    world.worldId = b2CreateWorld(&worldDef);
    fluid.init();

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
    groundBodyDef2.position.y = -100.0f;
    groundBodyDef2.type = b2_staticBody;
    b2BodyId groundId2 = b2CreateBody(world.worldId, &groundBodyDef2);
    b2Polygon groundBox2 = b2MakeBox(500.f, 10.0f);
    b2ShapeDef groundShapeDef2 = b2DefaultShapeDef();
    groundShapeDef2.density = 0.0f;
    b2CreatePolygonShape(groundId2, &groundShapeDef2, &groundBox2);

    b2BodyDef groundBodyDef3 = b2DefaultBodyDef();
    groundBodyDef3.position.x = 75.f;
    groundBodyDef3.position.y = -500.f;
    groundBodyDef3.type = b2_staticBody;
    b2BodyId groundId3 = b2CreateBody(world.worldId, &groundBodyDef3);
    b2Polygon groundBox3 = b2MakeBox(500.f, 10.f);
    b2ShapeDef groundShapeDef3 = b2DefaultShapeDef();
    groundShapeDef3.density = 0.0f;
    b2CreatePolygonShape(groundId3, &groundShapeDef3, &groundBox3);

    float timeStep = 0.2f;
    int subStepCount = 10;
    renderB2::RenderSettings rendersettings;
    rendersettings.OutlineThickness = 1;
    rendersettings.OutlineColor = sf::Color::White;
    rendersettings.FillColor = renderB2::DefaultColors::B2BodyFill;
    rendersettings.verticecount = 4;

    sf::Texture BackGroundT("Assets\\Textures\\BackGround.png");
    sf::Texture Snake("Assets\\Textures\\snake.png");
    BackGroundT.setRepeated(true);
    sf::RectangleShape BackGround({ (float)width - 350 * 2, (float)height - 25.f * 2 });
    BackGround.move({ 350.f, 25.f });
    BackGround.setOutlineThickness(3);
    BackGround.setOutlineColor(renderB2::DefaultColors::BackGroundOutline);
    BackGround.setTexture(&BackGroundT);
    BackGround.setTextureRect(sf::IntRect{ { 0, 0 }, { (int)(BackGround.getSize().x / 1.5), (int)(BackGround.getSize().y / 1.5) } });
    /*
    //smoke
    {
        fluid.Config.FORCE_SURFACE = 0.f;
        fluid.Config.Impact = 5.f;
        fluid.Config.FORCE_MULTIPLIER = -100.f;
    }
    */

    sf::RectangleShape* rect = new sf::RectangleShape(sf::Vector2f(400.f, 400.f));
    rect->setFillColor(renderB2::DefaultColors::TextBox);
    renderB2::MyUIObject* root = new renderB2::MyUIObject(rect, { 2000, 0 }, particleCount);
    sf::Text* awa = new sf::Text(renderB2::getDefaultFontAddress());
    awa->setString(std::to_string(fluid.Config.FORCE_SURFACE));
    awa->setFillColor(sf::Color::Black);
    awa->setStyle(sf::Text::Bold);
    awa->setCharacterSize(30.f);
    awa->setFillColor(sf::Color::White);
    renderB2::MyUIObject* child = new renderB2::MyUIObject(awa, sf::Vector2f(50.f, 0.f));
    root->addChild(child);

    sf::RectangleShape* buttonShape = new sf::RectangleShape(sf::Vector2f(50.f, 50.f));
    renderB2::UIButton* lbutton = new renderB2::UIButton(buttonShape, sf::Vector2f(0.f, 0.f));
    renderB2::UIButton* rbutton = new renderB2::UIButton(buttonShape, sf::Vector2f(150.f, 0.f));
    root->addChild(lbutton);
    root->addChild(rbutton);
    sf::RectangleShape* sliderShape = new sf::RectangleShape(sf::Vector2f(500.f, 50.f));
    sliderShape->setOutlineColor(renderB2::DefaultColors::BackGroundOutline);
	sliderShape->setOutlineThickness(3.f);
    sf::RectangleShape* buttonShape2 = new sf::RectangleShape(sf::Vector2f(50.f, 50.f));
    buttonShape2->setFillColor(sf::Color::Red);
    rbutton->onHover = [rbutton]() {
        sf::RectangleShape* buttonShape2 = new sf::RectangleShape(sf::Vector2f(50.f, 50.f));
        buttonShape2->setFillColor(sf::Color::Green);
        rbutton->shape = buttonShape2;
        };
    rbutton->onClick = [rbutton]() {
        sf::RectangleShape* buttonShape2 = new sf::RectangleShape(sf::Vector2f(50.f, 50.f));
        buttonShape2->setFillColor(sf::Color::Red);
        rbutton->shape = buttonShape2;
        fluid.Config.PLASTICITY_RATE += 1.f;
        rbutton->ClickSound.play();
        };
    rbutton->onRelease = [rbutton]() {
        rbutton->ReleaseSound.play();
        };

    lbutton->onHover = [lbutton]() {
        sf::RectangleShape* buttonShape2 = new sf::RectangleShape(sf::Vector2f(50.f, 50.f));
        buttonShape2->setFillColor(sf::Color::Green);
        lbutton->shape = buttonShape2;
        };
    lbutton->onClick = [lbutton]() {
        sf::RectangleShape* buttonShape2 = new sf::RectangleShape(sf::Vector2f(50.f, 50.f));
        buttonShape2->setFillColor(sf::Color::Red);
        lbutton->shape = buttonShape2;
        fluid.Config.PLASTICITY_RATE -= 1.f;
        lbutton->ClickSound.play();
        };
    lbutton->onRelease = [lbutton]() {
        lbutton->ReleaseSound.play();
        };
    renderB2::UISlider* slider = new renderB2::UISlider(sliderShape, buttonShape2, 0.f, 500.f, fluid.Config.FORCE_SURFACE, sf::Vector2f(0.f, 0.f),
         0.f, sf::Vector2f(1.f, 1.f));
    sf::Text* awa2 = new sf::Text(renderB2::getDefaultFontAddress());
    awa2->setString(std::to_string(slider->Value));
    awa2->setFillColor(sf::Color::Black);
    awa2->setStyle(sf::Text::Bold);
    awa2->setCharacterSize(30.f);
    awa2->setFillColor(sf::Color::Black);
    renderB2::MyUIObject* child2 = new renderB2::MyUIObject(awa2, sf::Vector2f(50.f, 0.f));
    slider->addChild(child2);
    slider->localPosition = { 50.f, 50.f };
	slider->onValueChange = [slider, awa2]() {
        std::ostringstream oss2;
        oss2 << std::fixed << std::setprecision(2) << slider->Value;
        awa2->setString(oss2.str());
		fluid.Config.FORCE_SURFACE = slider->Value;
		};

    while (window.isOpen()) {
        window.clear(renderB2::DefaultColors::ClearFill);
        elapsed = timeclock.restart();
        elapsedTime = elapsed.asMilliseconds();
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
            if (const auto* mouseReleased = event->getIf<sf::Event::MouseButtonReleased>()) {
                if (mouseReleased->button == sf::Mouse::Button::Left && selection.isSelecting) {
                    sf::Vector2f worldStart = window.mapPixelToCoords(sf::Vector2i(selection.startPos));
                    sf::Vector2f worldEnd = window.mapPixelToCoords(sf::Vector2i(selection.currentPos));

                    b2Vec2 start = MathUtils::toB2Position(worldStart.x, worldStart.y, worldView);
                    b2Vec2 end = MathUtils::toB2Position(worldEnd.x, worldEnd.y, worldView);

                    float minX = std::min(start.x, end.x);
                    float maxX = std::max(start.x, end.x);
                    float minY = std::min(start.y, end.y);
                    float maxY = std::max(start.y, end.y);
                    float Xlen = std::abs(maxX - minX);
                    float Ylen = std::abs(maxY - minY);
                    const float gridSize = 125;//water
                    //const float gridSize = 75;
                    float stepX = (maxX - minX) / (Xlen * gridSize / window.getSize().x);
                    float stepY = (maxY - minY) / (Ylen * gridSize / window.getSize().y);
                    std::cout << "selection : x:" << (Xlen * gridSize / window.getSize().x) << "y:" << (Ylen * gridSize / window.getSize().y) << std::endl;
                    for (int i = 0; i < (Xlen * gridSize / window.getSize().x); ++i) {
                        for (int j = 0; j < (Ylen * gridSize / window.getSize().y); ++j) {
                            float x = minX + i * stepX + stepX / 2;
                            float y = minY + j * stepY + stepY / 2;
                            //fluid.CreateParticle(world, -0.1, 4, x - camX, y + camY, 1.0f, 0.f, 0.1f); // smoke
                            fluid.CreateParticle(world, 1, 4, x - camX, y + camY, 2.5f, 0.f, 0.1f); // water
                            particleCount++;
                        }
                    }
                    selection.isSelecting = false;
                }
            }
            if (const auto* mouseMoved = event->getIf<sf::Event::MouseMoved>()) {
                if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Middle)) {
                    worldView.move({ (int)lastmousePos.x - (float)mouseMoved->position.x, (int)lastmousePos.y - (float)mouseMoved->position.y });
                }
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
                if (keyPressed->scancode == sf::Keyboard::Scan::F)
                {
                    fluid.freeze();
                }
                if (keyPressed->scancode == sf::Keyboard::Scan::G)
                {
                    fluid.unfreeze();
                }
            }
        }
        mousePos = sf::Mouse::getPosition(window);
        worldPos = MathUtils::toB2Position(mousePos.x, mousePos.y, worldView);



        //std::cout << worldPos.x << " " << worldPos.y << std::endl;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num2)) {
            particleCount++;
            //fluid.CreateParticle(world, -0.1, 4, worldPos.x - camX, worldPos.y + camY, 1.0f, 0.f, 0.1f);// smoke
            fluid.CreateParticle(world, 1, 4, worldPos.x - camX, worldPos.y + camY, 2.5f, 0.f, 0.1f);// water
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num1)) {
            createSquare(worldPos.x - camX, worldPos.y + camY);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R)) {
            for (auto& [bodyId, shape] : squares)
                b2DestroyBody(bodyId);
            for (auto& p : fluid.Particles)
				fluid.DestroyParticle(world, &p);
            squares.clear();
            fluid.Particles.clear();
            particleCount = 0;
            std::cout << "All deleted." << std::endl;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift)) {
            selection.isSelecting = true;
            selection.startPos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
            selection.currentPos = selection.startPos;
        }

        b2ExplosionDef Dragdef = b2DefaultExplosionDef();

        if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
            Drag = true;
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            Dragdef.position = b2Vec2{ worldPos.x - camX, worldPos.y + camY };
            Dragdef.radius = 100.0f;
            Dragdef.falloff = 5.f;
            Dragdef.impulsePerLength = -70.0f;
            b2World_Explode(world.worldId, &Dragdef);
        }

        if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Right)) {
            Drag = true;
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            Dragdef.position = b2Vec2{ worldPos.x - camX, worldPos.y + camY };
            Dragdef.radius = 50.0f;
            Dragdef.falloff = 0.f;
            Dragdef.impulsePerLength = 100.0f;
            b2World_Explode(world.worldId, &Dragdef);
        }
        if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Right) && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl)) {
            b2BodyDef bodyDef = b2DefaultBodyDef();
            bodyDef.type = b2_staticBody;
            bodyDef.position = worldPos;
            b2BodyId bodyId = b2CreateBody(world.worldId, &bodyDef);

            b2Polygon square = b2MakeBox(10.0f, 10.0f);
            b2ShapeDef shapeDef = b2DefaultShapeDef();
            b2CreatePolygonShape(bodyId, &shapeDef, &square);
            squares.push_back({ bodyId, square });
        }
        if (!PauseWorld) {
            for (auto it = fluid.Particles.begin(); it != fluid.Particles.end(); ) {
                it->nextTickLinearImpulse = b2Vec2_zero;
                it->nextTickForce = b2Vec2_zero;
                it->pos = b2Body_GetPosition(it->bodyId);
                it->LinearVelocity = b2Body_GetLinearVelocity(it->bodyId);
                if (it->age >= it->life && it->life >= 0) {
                    fluid.DestroyParticle(world, &(*it));
                }
                else {
                    it->age++;
                    ++it;
                }
            }
            fluid.ComputeParticleForces();
            for (const auto& p : fluid.Particles) {
                b2Body_ApplyLinearImpulseToCenter(p.bodyId, p.nextTickLinearImpulse, true);
                b2Body_ApplyForceToCenter(p.bodyId, p.nextTickForce, true);
            }
            b2World_Step(world.worldId, timeStep, subStepCount);
            fluid.UpdateDynamicTree();
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
        }


        window.setView(worldView);
        BackGround.setTextureRect(sf::IntRect{
            {
                 (int)-(BackGround.getSize().x / 3),
                 (int)-(BackGround.getSize().y / 3)
            },
            {
                 (int)(BackGround.getSize().x / 1.5),
                 (int)(BackGround.getSize().y / 1.5)
            }
            });
        window.setView(uiView);
        window.draw(BackGround);
        window.setView(worldView);

        renderB2::ScreenSettings screensettings;
        screensettings.camX = camX;
        screensettings.camY = camY;
        screensettings.width = width;
        screensettings.height = height;
        renderB2::renderb2Polygon(&window, rendersettings, groundBox, groundId, screensettings);
        renderB2::renderb2Polygon(&window, rendersettings, groundBox2, groundId2, screensettings);
        renderB2::renderb2Polygon(&window, rendersettings, groundBox3, groundId3, screensettings);

        for (auto& [bodyId, shape] : squares) {
            renderB2::renderb2Polygon(&window, rendersettings, shape, bodyId, screensettings);
        }

        rendersettings.verticecount = 3;
        renderB2::rendersimpleparticle(&window, rendersettings, fluid, screensettings);

        rendersettings.verticecount = 4;
        for (const auto& p : fluid.Particles) {
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
                        sf::CircleShape shape(2.5f, 32);
                        shape.setFillColor(sf::Color::Red);
                        shape.setOrigin({ 2.5f, 2.5f });
                        shape.setPosition(MathUtils::getSFpos(pos.x, pos.y));
                        //window.draw(shape);
                    }
                }
            }

            for (const auto& joint : p.AdhesionJoint)
            {
                if (B2_IS_NULL(joint))
                {
                    continue;
                }

                b2Vec2 force = b2Joint_GetConstraintForce(joint);
                b2Vec2 anchorA = b2Joint_GetLocalAnchorA(joint);// +b2Body_GetPosition(b2Joint_GetBodyA(p.AdhesionJoint[i]));
                b2Vec2 anchorB = b2Joint_GetLocalAnchorB(joint);// +b2Body_GetPosition(b2Joint_GetBodyB(p.AdhesionJoint[i]));
				//std::cout << anchorB.x << " " << anchorB.y << std::endl;
                std::array line =
                {
                    sf::Vertex{MathUtils::getSFpos(anchorA.x, anchorA.y)},
                    sf::Vertex{MathUtils::getSFpos(anchorB.x, anchorB.y)}
                };
                window.draw(line.data(), line.size(), sf::PrimitiveType::Lines);
            }
        }
        window.setView(uiView);
        if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) || sf::Mouse::isButtonPressed(sf::Mouse::Button::Right)) {
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
            selectionBox.setTexture(&Snake);
            selectionBox.setSize(sf::Vector2f(std::abs(size.x), std::abs(size.y)));
            selectionBox.setPosition({ std::min(selection.startPos.x, selection.currentPos.x),
                std::min(selection.startPos.y, selection.currentPos.y) });
            selectionBox.setFillColor(sf::Color(100, 100, 255, 50));
            selectionBox.setOutlineColor(sf::Color::Blue);
            selectionBox.setOutlineThickness(2);
            window.draw(selectionBox);
        }
        {
            sf::RectangleShape backgroundBorder = BackGround;
            backgroundBorder.move({ -BackGround.getOutlineThickness(), -BackGround.getOutlineThickness() });
            backgroundBorder.setSize({ BackGround.getSize().x + BackGround.getOutlineThickness() * 2, BackGround.getSize().y + BackGround.getOutlineThickness() * 2 });
            backgroundBorder.setFillColor(sf::Color::Transparent);
            backgroundBorder.setOutlineColor(renderB2::DefaultColors::ClearFill);
            backgroundBorder.setOutlineThickness(1000.0f);
            window.draw(backgroundBorder);
        }

        RenderUi();

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) {
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(2) << fluid.Config.PLASTICITY_RATE;
            awa->setString(oss.str());

            lbutton->shape = buttonShape;
            rbutton->shape = buttonShape;

            lbutton->tick(window);
            rbutton->tick(window);
            slider->tick(window);
            root->drawShadow(&window, 5.f, 0.f, sf::Color::Blue);
            root->draw(&window);
            slider->draw(&window);
        }

        window.display();
        lastmousePos = mousePos;
        tickCount++;
    }

    b2DynamicTree_Destroy(&fluid.dynamicTree);
    b2DestroyWorld(world.worldId);
    return 0;
}