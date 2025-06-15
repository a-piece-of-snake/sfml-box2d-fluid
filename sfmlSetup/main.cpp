
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
#include "ThreadPool.h"
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
sf::Vector2f mousePos;
sf::Vector2f lastmousePos;
b2Vec2 worldPos;

sf::Clock msclock;
int tickCount = 0;
sf::Clock tpsClock;
float tps = 0.0f;
sf::Time elapsed;
float elapsedTime;

bool Drag = false;
GameObjects::World world;
struct SelectionBox {
    bool isSelecting = false;
    sf::Vector2f startPos;
    sf::Vector2f currentPos;
} selection;

struct QueryContext
{
    b2Vec2 point;
    b2BodyId bodyId = b2_nullBodyId;
};

bool QueryCallback(b2ShapeId shapeId, void* context)
{
    QueryContext* queryContext = static_cast<QueryContext*>(context);

    b2BodyId bodyId = b2Shape_GetBody(shapeId);
    b2BodyType bodyType = b2Body_GetType(bodyId);
    if (bodyType != b2_dynamicBody)
    {
        // continue query
        return true;
    }

    bool overlap = b2Shape_TestPoint(shapeId, queryContext->point);
    if (overlap)
    {
        // found shape
        queryContext->bodyId = bodyId;
        return false;
    }

    return true;
}
bool PauseWorld = false;
int renderParticle = 1;
std::vector<std::pair<b2BodyId, b2Polygon>> squares;
std::vector<std::pair<b2BodyId, b2Circle>> circles;
//std::vector<GameObjects::Particle> particles;
GameObjects::ParticleGroup fluid;
sf::Texture BackGroundT("Assets\\Textures\\BackGround.png");
sf::Texture Snake("Assets\\Textures\\snake.png");
sf::Texture smoothFunctionT("Assets\\Textures\\SmoothFunction.png");

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


void createSquare(float x, float y, float density, float friction, float restitution, float size) {
    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = b2_dynamicBody;
    bodyDef.position = b2Vec2{ x, y };
    b2BodyId bodyId = b2CreateBody(world.worldId, &bodyDef);

    b2Polygon square = b2MakeBox(size, size);
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.density = density;
    shapeDef.friction = friction;
    shapeDef.restitution = restitution;
    b2CreatePolygonShape(bodyId, &shapeDef, &square);
    squares.push_back({ bodyId, square });
}
void createCircle(float x, float y, float density, float friction, float restitution, float size) {
    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = b2_dynamicBody;
    bodyDef.position = b2Vec2{ x, y };
    bodyDef.fixedRotation = true;
    b2BodyId bodyId = b2CreateBody(world.worldId, &bodyDef);

    b2Circle circle;
    circle.center = b2Vec2{ 0.0f, 0.0f };
    circle.radius = size;
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.density = density;
    shapeDef.friction = friction;
    shapeDef.restitution = restitution;
	b2CreateCircleShape(bodyId, &shapeDef, &circle);
    circles.push_back({ bodyId, circle });
}
void RenderUi() {
    {//左边ui
        sf::RectangleShape leftUIshape({ 350.f - 8.f,  (float)height - 225.f });
        leftUIshape.setFillColor(sf::Color::Transparent);
        renderB2::MyUIObject* leftUI = new renderB2::MyUIObject(&leftUIshape, sf::Vector2f(8.f, 0.f + 200.f));
        leftUI->lineSpacing = 5.f;
        //{
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
            //root->localPosition = { 8.f, 25.f };
            //root->draw(&window);
            leftUI->addChild(root);
        //}
        //{
            if (tpsClock.getElapsedTime().asSeconds() >= 1.0f) {
                tps = tickCount / tpsClock.getElapsedTime().asSeconds();
                tickCount = 0;
                tpsClock.restart();
            }
            sf::ConvexShape tpsshape = renderB2::getRectangleMinusCorners(330.f, 115.f, 11.5f);
            tpsshape.setFillColor(sf::Color::Transparent);
            tpsshape.setOutlineThickness(3.f);
            renderB2::MyUIObject* root2 = new renderB2::MyUIObject(&tpsshape, { 0, 0 }, 0);
            sf::Text awa2(renderB2::getDefaultFontAddress());
            awa2.setString("TPS\n" + std::to_string((int)tps));
            awa2.setStyle(sf::Text::Bold);
            awa2.setCharacterSize(30.f);
            if (tps <= 10) {
                tpsshape.setOutlineColor(renderB2::DefaultColors::WarningOutline);
                awa2.setFillColor(renderB2::DefaultColors::WarningText);
            }
            else {
                tpsshape.setOutlineColor(sf::Color::White);
                awa2.setFillColor(sf::Color::White);
            }
            awa2.setOrigin({ awa2.getLocalBounds().size.x / 2, awa2.getLocalBounds().size.y / 2 });
            renderB2::MyUIObject* child2 = new renderB2::MyUIObject(&awa2, sf::Vector2f(50.f, 0.f));
            child2->localPosition = { tpsshape.getLocalBounds().size.x / 2, tpsshape.getLocalBounds().size.y / 2 };
            root2->addChild(child2);
            //root2->localPosition = { 8.f, 151.f };
            //root2->draw(&window);
            leftUI->addChild(root2);
        //}
        //{
            if (tpsClock.getElapsedTime().asSeconds() >= 1.0f) {
                tps = tickCount / tpsClock.getElapsedTime().asSeconds();
                tickCount = 0;
                tpsClock.restart();
            }
            sf::ConvexShape particlecount = renderB2::getRectangleMinusCorners(330.f, 115.f, 11.5f);
            particlecount.setFillColor(sf::Color::Transparent);
            particlecount.setOutlineColor(sf::Color::White);
            particlecount.setOutlineThickness(3.f);
            renderB2::MyUIObject* root3 = new renderB2::MyUIObject(&particlecount, { 0, 0 }, 0);
            sf::Text awa3(renderB2::getDefaultFontAddress());
            awa3.setString("Particle Count\n     " + std::to_string(particleCount));
            awa3.setStyle(sf::Text::Bold);
            awa3.setCharacterSize(30.f);
            awa3.setFillColor(sf::Color::White);
            awa3.setOrigin({ awa3.getLocalBounds().size.x / 2, awa3.getLocalBounds().size.y / 2 });
            renderB2::MyUIObject* child3 = new renderB2::MyUIObject(&awa3, sf::Vector2f(50.f, 0.f));
            child3->localPosition = { particlecount.getLocalBounds().size.x  / 2, particlecount.getLocalBounds().size.y / 2 };
            root3->addChild(child3);
            //root->localPosition = { 8.f, 277.f };
            //root->draw(&window);
		    leftUI->addChild(root3);
            leftUI->draw(&window);
        //}
        //{
            /*
            sf::RectangleShape tutorial;
            tutorial.setSize({ 330.f, 330.f });
            tutorial.setFillColor(renderB2::DefaultColors::TextBox);
            tutorial.setOutlineThickness(3.f);
            tutorial.setOutlineColor(sf::Color::White);
            renderB2::MyUIObject* root4 = new renderB2::MyUIObject(&tutorial, { 0, 0 }, 0);
            sf::Text awa4(renderB2::getDefaultFontAddress());
            awa4.setString("LShift: Drag-select\narea to create\nparticles\nNum1: Box\nNum2: Particle\nRMB: Clear\nSpace: Pause");
            awa4.setStyle(sf::Text::Bold);
            awa4.setCharacterSize(30.f);
            awa4.setFillColor(sf::Color::White);
            awa4.setOrigin( { awa4.getLocalBounds().size.x / 2, awa4.getLocalBounds().size.y / 2 } );
            renderB2::MyUIObject* child4 = new renderB2::MyUIObject(&awa4, sf::Vector2f(50.f, 0.f));
            child4->localPosition = { tutorial.getLocalBounds().size.x / 2, tutorial.getLocalBounds().size.y / 2 };
            root4->addChild(child4);
            //root->localPosition = { 8.f, 403 };
            //root->draw(&window);
            leftUI->addChild(root4);
        //}
        leftUI->draw(&window);
    }
    {//顶上的ui
        {//标题
            float titleWidth = 1000.f;
            float titleHeight = 175.f;
            sf::RectangleShape* bg = new sf::RectangleShape({ titleWidth, titleHeight });
            bg->setFillColor(renderB2::DefaultColors::Button);
            bg->setOutlineThickness(2.f);
            bg->setOutlineColor(sf::Color::White);
            float bgX = (width - titleWidth) / 2.f;
            float bgY = 10.f;
            renderB2::MyUIObject* titleUI = new renderB2::MyUIObject(bg, { bgX, bgY });
            sf::RectangleShape* leftBlock = new sf::RectangleShape({ 50.f, titleHeight });
            leftBlock->setFillColor(renderB2::DefaultColors::BackGroundOutline);
            renderB2::MyUIObject* leftUI = new renderB2::MyUIObject(leftBlock, { 0.f, 0.f });
            leftUI->lineBreak = false;
            titleUI->addChild(leftUI);
            sf::RectangleShape* rightBlock = new sf::RectangleShape({ 50.f, titleHeight });
            rightBlock->setFillColor(renderB2::DefaultColors::BackGroundOutline);
            renderB2::MyUIObject* rightUI = new renderB2::MyUIObject(rightBlock, { titleWidth - 50.f, 0.f });
            rightUI->lineBreak = false;
            titleUI->addChild(rightUI);
            sf::Text* text = new sf::Text(renderB2::getDefaultFontAddress());
            text->setString(L"Position Based Fluid");
            text->setCharacterSize(70);
            text->setFillColor(sf::Color::White);
            auto bounds = text->getLocalBounds();
            text->setOrigin({ bounds.size.x / 2.f, bounds.size.y / 2.f });
            renderB2::MyUIObject* textUI = new renderB2::MyUIObject(text, { titleWidth / 2.f, titleHeight / 2.f });
            textUI->lineBreak = false;
            titleUI->addChild(textUI);
            titleUI->draw(&window); 
        }
        {//右边的ui
            sf::RectangleShape rightUIshape({ 350.f - 8.f,  (float)height - 225.f });
            rightUIshape.setFillColor(sf::Color::Transparent);
            renderB2::MyUIObject* rightUI = new renderB2::MyUIObject(&rightUIshape, sf::Vector2f(8.f + mousePos.x, 0.f + 200.f + mousePos.y));
            rightUI->lineSpacing = 5.f;
            //{
                float diameter = 330;
                float radius = diameter / 2;
                float width = 2.f;
                sf::RectangleShape particlebackground({ diameter, diameter });
                particlebackground.setOutlineThickness(3);
                particlebackground.setOrigin({ radius, radius });
                particlebackground.setOutlineColor(renderB2::DefaultColors::BackGroundOutline);
                particlebackground.setTexture(&BackGroundT);
                particlebackground.setTextureRect(sf::IntRect{
                    {
                         (int)-(particlebackground.getLocalBounds().size.x / 4),
                         (int)-(particlebackground.getLocalBounds().size.y / 4)
                    },
                    {
                         (int)(particlebackground.getLocalBounds().size.x / 2),
                         (int)(particlebackground.getLocalBounds().size.y / 2)
                    }
                    });

                sf::CircleShape smoothRadiusCircle(radius / 1.5f, 32);
                smoothRadiusCircle.setOutlineThickness(3);
                smoothRadiusCircle.setOrigin({ radius / 1.5f, radius / 1.5f });
                smoothRadiusCircle.setOutlineColor(renderB2::DefaultColors::TextBox);
                smoothRadiusCircle.setFillColor(renderB2::DefaultColors::LightBlue);
                sf::CircleShape particle(radius / 30, 32);
                particle.setFillColor(renderB2::DefaultColors::B2BodyFill);
                particle.setOutlineColor(sf::Color::White);
                particle.setOutlineThickness(3.f);
                particle.setOrigin({ radius / 30, radius / 30 });
			    sf::RectangleShape smoothRadius({ width, radius / 1.5f });
                smoothRadius.setFillColor(sf::Color::White);
                //smoothRadius.setOutlineThickness(3.f);
                //smoothRadius.setOutlineColor(sf::Color::White);
                renderB2::MyUIObject* particlebackgroundUI = new renderB2::MyUIObject(&particlebackground, { radius, radius }); //mousePos);
                renderB2::MyUIObject* smoothRadiusCircleUI = new renderB2::MyUIObject(&smoothRadiusCircle, { 0, 0 });
                smoothRadiusCircleUI->lineBreak = false;
                renderB2::MyUIObject* particleUI = new renderB2::MyUIObject(&particle, { 0, 0 });
                particleUI->lineBreak = false;
                smoothRadiusCircleUI->addChild(particleUI);
                renderB2::MyUIObject* smoothRadiusUI = new renderB2::MyUIObject(&smoothRadius, { -width / 2, -radius / 1.5f });
                smoothRadiusUI->lineBreak = false;
                sf::Text smoothRadiustext(renderB2::getDefaultFontAddress());
                std::ostringstream oss2;
                oss2 << std::fixed << std::setprecision(2) << fluid.Config.radius * fluid.Config.Impact;
                smoothRadiustext.setString(oss2.str());
                smoothRadiustext.setCharacterSize(20);
                smoothRadiustext.setFillColor(sf::Color::White);
                renderB2::MyUIObject* smoothRadiustextUI = new renderB2::MyUIObject(&smoothRadiustext, { -width / 2, -radius / 3.f - smoothRadiustext.getLocalBounds().size.y / 2});
                smoothRadiusUI->lineBreak = false;
                particlebackgroundUI->addChild(smoothRadiusCircleUI);
                particlebackgroundUI->addChild(smoothRadiusUI);
                particlebackgroundUI->addChild(smoothRadiustextUI);
                rightUI->addChild(particlebackgroundUI);
            //}
            //{
				sf::RectangleShape smoothFunction({ diameter, radius });
				smoothFunction.setOutlineThickness(3);
                smoothFunction.setOutlineColor(renderB2::DefaultColors::BackGroundOutline);
                smoothFunction.setTexture(&smoothFunctionT);
                renderB2::MyUIObject* smoothFunctionUI = new renderB2::MyUIObject(&smoothFunction, { 0, 0 });
                rightUI->addChild(smoothFunctionUI);
                
            //}
                rightUI->draw(&window);
        }*/
    }
}
struct B2ThreadContext {
    ThreadPool* pool;
};

// Box2D 要求你返回一个 “任务句柄”，可以是 future、vector、或者 nullptr
void* MyEnqueueTask(
    void (*task)(int startIndex, int endIndex, uint32_t workerIndex, void* taskContext),
    int itemCount,
    int minRange,
    void* taskContext,
    void* userContext)
{
    B2ThreadContext* ctx = static_cast<B2ThreadContext*>(userContext);
    ThreadPool* pool = ctx->pool;

    int workerCount = std::thread::hardware_concurrency();
    int chunkSize = std::max(minRange, (itemCount + workerCount - 1) / workerCount);

    auto* handles = new std::vector<std::future<void>>();

    for (int i = 0; i < itemCount; i += chunkSize) {
        int start = i;
        int end = std::min(i + chunkSize, itemCount);
        uint32_t workerIndex = i / chunkSize;

        handles->emplace_back(
            pool->enqueue([=]() {
                task(start, end, workerIndex, taskContext);
                })
        );
    }

    return handles;
}

void MyFinishTask(void* taskHandle, void* userContext) {
    auto* handles = static_cast<std::vector<std::future<void>>*>(taskHandle);
    for (auto& f : *handles) f.wait();
    delete handles;
}


int main() {
   window.setFramerateLimit(100);
   const sf::Image Icon("Assets\\Textures\\water2d.png");
   window.setIcon(Icon);
    //static ThreadPool particlePool(std::thread::hardware_concurrency());
    sf::View view(sf::FloatRect({ 0.f, 0.f }, { (float)width, (float)height }));
    window.setView(view);

    ThreadPool myThreadPool(std::thread::hardware_concurrency());
    B2ThreadContext b2Ctx;
    b2Ctx.pool = &myThreadPool;


    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = b2Vec2{ -2.5f, 0.0f };
    //worldDef.gravity = b2Vec2{ 0.0f, 0.0f };
    worldDef.contactHertz = 60.f;
    worldDef.enableContinuous = false;
    worldDef.workerCount = std::thread::hardware_concurrency();
    worldDef.enqueueTask = MyEnqueueTask;
    worldDef.finishTask = MyFinishTask;
    worldDef.userTaskContext = &b2Ctx;
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
    groundBodyDef2.position.x = 400.f;
    groundBodyDef2.position.y = 600.0f;
    groundBodyDef2.type = b2_staticBody;
    b2BodyId groundId2 = b2CreateBody(world.worldId, &groundBodyDef2);
    b2Polygon groundBox2 = b2MakeBox(1000.f, 10.0f);
    b2ShapeDef groundShapeDef2 = b2DefaultShapeDef();
    groundShapeDef2.density = 0.0f;
    b2CreatePolygonShape(groundId2, &groundShapeDef2, &groundBox2);

    b2BodyDef groundBodyDef3 = b2DefaultBodyDef();
    groundBodyDef3.position.x = 400.f;
    groundBodyDef3.position.y = -600.f;
    groundBodyDef3.type = b2_staticBody;
    b2BodyId groundId3 = b2CreateBody(world.worldId, &groundBodyDef3);
    b2Polygon groundBox3 = b2MakeBox(1000.f, 10.f);
    b2ShapeDef groundShapeDef3 = b2DefaultShapeDef();
    groundShapeDef3.density = 0.0f;
    b2CreatePolygonShape(groundId3, &groundShapeDef3, &groundBox3);

    b2BodyDef playerDef = b2DefaultBodyDef();//定义玩家
    playerDef.fixedRotation = true;
    playerDef.isBullet = true;
    playerDef.position = { 100, 100 };
    playerDef.type = b2_dynamicBody;
    //playerDef.gravityScale = 0.f;
    b2BodyId playerId = b2CreateBody(world.worldId, &playerDef);
    b2ShapeDef playerShapeDef = b2DefaultShapeDef();
    playerShapeDef.density = 2.f;
    //playerShapeDef.friction = 0.f;
    playerShapeDef.friction = 0.5f;
    playerShapeDef.restitution = 0.f;
    b2Circle playerCircle;
    playerCircle.radius = 10;
    playerCircle.center = b2Vec2{ 0.0f, 0.0f };
    b2ShapeId playerShapeId = b2CreateCircleShape(playerId, &playerShapeDef, &playerCircle);

    float timeStep = 0.2f;
    int subStepCount = 4;
    renderB2::RenderSettings rendersettings;
    rendersettings.OutlineThickness = 1;
    rendersettings.OutlineColor = sf::Color::White;
    rendersettings.FillColor = renderB2::DefaultColors::B2BodyFill;
    rendersettings.verticecount = 4;

    BackGroundT.setRepeated(true);
    sf::RectangleShape BackGround({ (float)width - 350 * 2, (float)height - 125.f * 2 });
    BackGround.move({ 350.f, 200.f });
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
    awa->setStyle(sf::Text::Bold);
    awa->setCharacterSize(30.f);
    awa->setFillColor(sf::Color::White);
    renderB2::MyUIObject* child = new renderB2::MyUIObject(awa, sf::Vector2f(50.f, 0.f));
    child->lineBreak = false;
    root->addChild(child);

    sf::RectangleShape* buttonShape = new sf::RectangleShape(sf::Vector2f(50.f, 50.f));
    buttonShape->setFillColor(renderB2::DefaultColors::Button);
    buttonShape->setOutlineThickness(3.f);
    buttonShape->setOutlineColor(renderB2::DefaultColors::BackGroundOutline);
    renderB2::UICheckBox* checkBox = new renderB2::UICheckBox(buttonShape, sf::Vector2f(0.f, 100.f), false);
    renderB2::UIButton* lbutton = new renderB2::UIButton(buttonShape, sf::Vector2f(0.f, 0.f));
    renderB2::UIButton* rbutton = new renderB2::UIButton(buttonShape, sf::Vector2f(150.f, 0.f));
    lbutton->lineBreak = false;
    rbutton->lineBreak = false;
    root->addChild(checkBox);
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
        fluid.Config.FORCE_SURFACE += 1.f;
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
        fluid.Config.FORCE_SURFACE -= 1.f;
        lbutton->ClickSound.play();
        };
    lbutton->onRelease = [lbutton]() {
        lbutton->ReleaseSound.play();
        };
    renderB2::UISlider* slider = new renderB2::UISlider(sliderShape, buttonShape2, 0.f, 200.f, fluid.Config.FORCE_SURFACE, sf::Vector2f(0.f, 0.f),
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
    renderB2::UISlider* slider2 = new renderB2::UISlider(sliderShape, buttonShape2, 0.f, 12.5f, fluid.Config.VISCOSITY, sf::Vector2f(0.f, 0.f),
        0.f, sf::Vector2f(1.f, 1.f));
    sf::Text* awa3 = new sf::Text(renderB2::getDefaultFontAddress());
    awa3->setString(std::to_string(slider2->Value));
    awa3->setFillColor(sf::Color::Black);
    awa3->setStyle(sf::Text::Bold);
    awa3->setCharacterSize(30.f);
    awa3->setFillColor(sf::Color::Black);
    renderB2::MyUIObject* child3 = new renderB2::MyUIObject(awa3, sf::Vector2f(50.f, 0.f));
    slider2->addChild(child3);
    slider2->localPosition = { 50.f, 200.f };
    slider2->onValueChange = [slider2, awa3]() {
        std::ostringstream oss2;
        oss2 << std::fixed << std::setprecision(2) << slider2->Value;
        awa3->setString(oss2.str());
        fluid.Config.SHEAR_VISCOSITY = slider2->Value;
        fluid.Config.VISCOSITY = slider2->Value;
        fluid.Config.VISCOSITY_LEAVE = slider2->Value / 10.f;
        };

    while (window.isOpen()) {
        window.clear(renderB2::DefaultColors::ClearFill);
        elapsed = msclock.restart();
        elapsedTime = elapsed.asMilliseconds();
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
            if (const auto* mousePressed = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (mousePressed->button == sf::Mouse::Button::Left) {
                    b2AABB box;
                    b2Vec2 d = { 0.001f, 0.001f };
					b2Vec2 p = MathUtils::toB2Position(mousePos.x, mousePos.y, worldView);
                    box.lowerBound = b2Sub(p, d);
                    box.upperBound = b2Add(p, d);

                    // Query the world for overlapping shapes.
                    QueryContext queryContext = { p, b2_nullBodyId };
                    b2World_OverlapAABB(world.worldId, box, b2DefaultQueryFilter(), QueryCallback, &queryContext);

                    if (B2_IS_NON_NULL(queryContext.bodyId))
                    {
                        b2BodyDef bodyDef = b2DefaultBodyDef();
                        world.groundBodyId = b2CreateBody(world.worldId, &bodyDef);

                        b2MouseJointDef mouseDef = b2DefaultMouseJointDef();
                        mouseDef.bodyIdA = world.groundBodyId;
                        mouseDef.bodyIdB = queryContext.bodyId;
                        mouseDef.target = p;
                        mouseDef.hertz = 5.0f;
                        mouseDef.dampingRatio = 0.7f;
                        mouseDef.maxForce = 1000.0f * b2Body_GetMass(queryContext.bodyId);
                        world.mouseJointId = b2CreateMouseJoint(world.worldId, &mouseDef);

                        b2Body_SetAwake(queryContext.bodyId, true);
                    }
                }
            }
            if (const auto* mouseReleased = event->getIf<sf::Event::MouseButtonReleased>()) {
                if (mouseReleased->button == sf::Mouse::Button::Left && selection.isSelecting) {
                    sf::Vector2f worldStart = selection.startPos;
                    sf::Vector2f worldEnd = selection.currentPos;
                   
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
                            //createSquare(x, y, 0.5, 0.3, 0.1, 5);
                            //fluid.CreateParticle(world, -0.1, 4, x, y, 1.0f, 0.f, 0.1f); // smoke
                            fluid.CreateParticle(world, 1, 4, x, y, 2.5f, 0.f, 0.1f, sf::Color::Cyan); // water
                            particleCount++;
                        }
                    }
                    selection.isSelecting = false;
                }
            }
            if (const auto* mouseMoved = event->getIf<sf::Event::MouseMoved>()) {
                if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Middle)) {
                    worldView.move({ (int)lastmousePos.x - (float)mouseMoved->position.x, (int)lastmousePos.y - (float)mouseMoved->position.y });
					camX = worldView.getCenter().x - worldView.getSize().x / 2;
					camY = worldView.getCenter().y - worldView.getSize().y / 2;
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
                if (keyPressed->scancode == sf::Keyboard::Scan::P)
                {
                    renderParticle = (renderParticle + 1) % 4;
                }
            }
        }
        mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
        worldPos = MathUtils::toB2Position(mousePos.x, mousePos.y, worldView);



        //std::cout << worldPos.x << " " << worldPos.y << std::endl;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num5)) {
            particleCount++;
            //fluid.CreateParticle(world, 1, 4, worldPos.x, worldPos.y, 2.5f, 0.5f, 0.1f, sf::Color::Cyan); // water
            fluid.CreateParticle(world, 1, 4, worldPos.x, worldPos.y, 2.5f, 0.f, 0.1f, sf::Color::Cyan); // water
        }
        //std::cout << worldPos.x << " " << worldPos.y << std::endl;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num4)) {
            particleCount++;
            //fluid.CreateParticle(world, -0.1, 4, worldPos.x, worldPos.y, 1.0f, 0.f, 0.1f);// smoke
            fluid.CreateParticle(world, 1, 3, worldPos.x, worldPos.y, 1.5f, 0.f, 0.1f, sf::Color::Red);// water
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num3)) {
            createCircle(worldPos.x, worldPos.y, 0.2, 0.25, 0.1, 5);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num2)) {
            createSquare(worldPos.x, worldPos.y, 0.1, 0.3, 0.1, 50);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num1)) {
            createSquare(worldPos.x, worldPos.y, 0.5, 0.3, 0.1, 5);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R)) {
            for (auto& [bodyId, shape] : squares)
                b2DestroyBody(bodyId);
            for (auto& [bodyId, shape] : circles)
                b2DestroyBody(bodyId);
            for (auto& p : fluid.Particles)
				fluid.DestroyParticle(world, &p);
            squares.clear();
            circles.clear();
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

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) {
            Drag = true;
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            Dragdef.position = b2Vec2{ worldPos.x, worldPos.y };
            Dragdef.radius = 100.0f;
            Dragdef.falloff = 5.f;
            Dragdef.impulsePerLength = -70.0f;
            b2World_Explode(world.worldId, &Dragdef);
        }

        if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Right)) {
            Drag = true;
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            Dragdef.position = b2Vec2{ worldPos.x, worldPos.y };
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

        {
            b2Vec2 force = b2Vec2_zero;
            force.x = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up) - sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down);
            force.y = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) - sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left);
            b2Body_SetLinearVelocity(playerId, force * 30.f);
        }
        /*
        {//控制玩家
            b2Vec2 force = b2Vec2_zero;
            b2Vec2 jumpforce = b2Vec2_zero;
            b2Vec2 ovelocity = b2Body_GetLinearVelocity(playerId);
            force.y = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) - sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left);
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) {
                std::vector<b2ContactData> contactData;
                int capacity = b2Shape_GetContactCapacity(playerShapeId);
                contactData.resize(capacity);
                int count = b2Body_GetContactData(playerId, contactData.data(), capacity);
                for (int i = 0; i < count; ++i) {
                    b2Manifold manifold = contactData[i].manifold;

                    b2BodyId bodyIdA = b2Shape_GetBody(contactData[i].shapeIdA);
                    b2BodyId bodyIdB = b2Shape_GetBody(contactData[i].shapeIdB);
                    std::string nameA = b2Body_GetName(bodyIdA);
                    std::string nameB = b2Body_GetName(bodyIdB);
                    for (int k = 0; k < manifold.pointCount; ++k) {
                        b2ManifoldPoint point = manifold.points[k];
                        b2Vec2 pos = point.point;
                        jumpforce += b2Normalize(pos - b2Body_GetPosition(playerId));
                    }
                }
            }
            jumpforce = b2Normalize(jumpforce) * -50.f;
            b2Body_SetLinearVelocity(playerId, (b2Vec2{ jumpforce.x, (force * 20.f).y + ovelocity.y / 2.f + jumpforce.y })  + b2World_GetGravity(world.worldId) * 0.f);
        }
        */

        if (!PauseWorld) {
            fluid.UpdateData(world);
            fluid.ComputeParticleForces();
            for (const auto& p : fluid.Particles) {
                b2Body_ApplyLinearImpulseToCenter(p.bodyId, p.nextTickLinearImpulse, true);
                b2Body_ApplyForceToCenter(p.bodyId, p.nextTickForce, true);
            }
            b2World_Step(world.worldId, timeStep, subStepCount);
        }


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
        renderB2::renderSoul(&window, playerCircle, playerId, screensettings);
        for (auto& [bodyId, shape] : squares) {
            renderB2::renderb2Polygon(&window, rendersettings, shape, bodyId, screensettings);
        }
        rendersettings.verticecount = 6;
        for (auto& [bodyId, shape] : circles) {
            renderB2::renderb2circle(&window, rendersettings, shape, bodyId, screensettings);
        }

        rendersettings.verticecount = 3;
        switch (renderParticle) {
        case 0:
            //renderB2::rendersimpleparticle(&window, rendersettings, fluid, screensettings);
            break;
        case 1:
            renderB2::rendersimpleparticle(&window, rendersettings, fluid, screensettings);
            break;
        case 2:
            renderB2::renderwater(&window, rendersettings, fluid, screensettings);
            break;
        case 3:
            renderB2::renderparticletest(&window, rendersettings, fluid, screensettings);
            break;
        default:
            std::cout << "RenderError" << std::endl;
        }
        std::vector<b2ContactData> contactData;
        int capacity = b2Shape_GetContactCapacity(playerShapeId);
        contactData.resize(capacity);
        int count = b2Body_GetContactData(playerId, contactData.data(), capacity);
        
        for (int i = 0; i < count; ++i) {
            b2Manifold manifold = contactData[i].manifold;

            b2BodyId bodyIdA = b2Shape_GetBody(contactData[i].shapeIdA);
            b2BodyId bodyIdB = b2Shape_GetBody(contactData[i].shapeIdB);
            std::string nameA = b2Body_GetName(bodyIdA);
            std::string nameB = b2Body_GetName(bodyIdB);
            for (int k = 0; k < manifold.pointCount; ++k) {
                b2ManifoldPoint point = manifold.points[k];
                b2Vec2 pos = point.point;
                sf::CircleShape shape(2.5f, 32);
                shape.setFillColor(sf::Color::Red);
                shape.setOrigin({ 2.5f, 2.5f });
                shape.setPosition(MathUtils::getSFpos(pos.x, pos.y));
                window.draw(shape);

            }
        }

        rendersettings.verticecount = 4;

        window.setView(uiView);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) || sf::Mouse::isButtonPressed(sf::Mouse::Button::Right)) {
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

            {
                sf::CircleShape starbackground(100, 32);
                starbackground.setOrigin({ 100, 100 });
                sf::Shader starShader;
                if (!starShader.loadFromFile("Assets\\Shaders\\star.frag", sf::Shader::Type::Fragment)) {
                    return EXIT_FAILURE;
                }

                starbackground.move({ (float)mousePos.x, (float)mousePos.y });
                starShader.setUniform("iResolution", sf::Vector2f(window.getSize()));
                starShader.setUniform("iTime", world.clock.getElapsedTime().asSeconds());
                window.draw(starbackground, &starShader);
            }

            std::ostringstream oss;
            oss << std::fixed << std::setprecision(2) << fluid.Config.FORCE_SURFACE;
            awa->setString(oss.str());

            lbutton->shape = buttonShape;
            rbutton->shape = buttonShape;
            checkBox->tick(window);
            lbutton->tick(window);
            rbutton->tick(window);
            slider->tick(window);
            slider2->tick(window);
            root->drawShadow(&window, 5.f, 0.f, sf::Color::Blue);
            root->draw(&window);
            slider->draw(&window);
            slider2->draw(&window);
        }

        window.display();
        lastmousePos = mousePos;
        tickCount++;
    }

    b2DestroyWorld(world.worldId);
    return 0;
}
