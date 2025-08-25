#pragma once

#include <iostream>
#include <vector>
#include <cstdlib>
#include <cmath>
#include <random>
#include <iomanip>
#include <functional>
#include <thread>
#include <numeric>
#include <sstream>

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
#include "ColorfulLog.h"

#include "imgui/imgui.h"
#include "imgui/imgui-SFML.h"
#include "imgui/imfilebrowser.h"

class Game {
public:
    Game();
    ~Game();

    void Init();
    void Run();
    void Destroy();

private:
    // 窗口和视图
    int width = 2400, height = 1350;
    sf::RenderWindow window;
    sf::View worldView;
    sf::View uiView;

    // 输入相关
    sf::Vector2f mousePos;
    sf::Vector2f lastmousePos;
    b2Vec2 worldPos = { 0,0 };

    // 时间/FPS
    sf::Clock msclock;
    sf::Clock fpsClock;
    sf::Clock deltaClock;
    int tickCount = 0;
    float fps = 0.0f;
    sf::Time elapsed;
    float elapsedTime = 0.0f;

    // 世界与对象
    bool PauseWorld = false;
    int renderParticle = 1;
    int particleCount = 0;
    GameObjects::World world;
    GameObjects::ParticleGroup fluid;
    std::vector<std::pair<b2BodyId, b2Polygon>> squares;
    std::vector<std::pair<b2BodyId, b2Circle>> circles;
    std::vector<b2BodyId> mapBodyId;
    bool Drag = false;
    b2ExplosionDef Dragdef = b2DefaultExplosionDef();
    // 相机
    float camX = 0.f, camY = 0.f;

    ImGui::FileBrowser fileDialog;

    // 纹理
    sf::Texture BackGroundT;
    sf::Texture Snake;
    sf::Texture smoothFunctionT;

    // 选择框
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

    // Box2D 多线程上下文
    struct B2ThreadContext {
        ThreadPool* pool;
    } b2Ctx;

    // 工具函数
    static bool QueryCallback(b2ShapeId shapeId, void* context);
    static void* MyEnqueueTask(
        void (*task)(int startIndex, int endIndex, uint32_t workerIndex, void* taskContext),
        int itemCount, int minRange, void* taskContext, void* userContext);
    static void MyFinishTask(void* taskHandle, void* userContext);

    // 游戏逻辑
    void createSquare(float x, float y, float density, float friction, float restitution, float size);
    void createCircle(float x, float y, float density, float friction, float restitution, float size);

    // 循环分解
	void ImGuiInit();
    void InitScene();
    void Update();
    void Render();
    void ImGuiRelated();
    void KeyLogic();
    void HandleEvent(const sf::Event& event);
    void UpdateWorld();
    void DrawWorld();
    void RenderUi();
    void RenderSKeyOverlay();
};
