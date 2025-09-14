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

#ifdef _WIN32
    #include <windows.h>
    #include <dwmapi.h>
#endif

#include "ResourceLoader.h"
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

    // 资源
	Resourses resources;

    // 输入相关
    sf::Vector2f mousePos;
    sf::Vector2f lastmousePos;
    b2Vec2 worldPos = { 0,0 };
    std::string userCommend;

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
    GameObjects::SpawnableObject* selectedObject[10] = { nullptr };
    float selectedObjectSize = 1.f;
    float selectedObjectFriction = 0.8f;
    float selectedObjectRestitution = 0.2f;
    std::vector<std::pair<b2BodyId, b2Polygon>> squares;
    std::vector<std::pair<b2BodyId, b2Circle>> circles;
    std::vector<b2BodyId> mapBodyId;
    bool Drag = false;
    b2ExplosionDef Dragdef = b2DefaultExplosionDef();

    // 相机
    float camX = 0.f, camY = 0.f;

    // ImGui
    ImGui::FileBrowser fileDialog;
	bool showConsole = false;
    float consoleAnimation = -40.f;
	GameObjects::SpawnableObjectBrowser spawnBrowser;

    // 纹理
    sf::Texture BackGroundT;
    sf::Texture Snake;
    sf::Texture smoothFunctionT;

    // 选择框
    struct SelectionBox {
        bool isSelecting = false;
        /*
        sf::Vector2f startPos;
        sf::Vector2f currentPos;*/
        b2Vec2 startPos;
        b2Vec2 currentPos;
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
    void createCup(float x, float y, float friction, float restitution, float width);
    // 循环分解
	void InitWindow();
	void InitImGui();
    void InitScene();
	void LoadResources();
    void Update();
    void Render();
    void ImGuiRelated();
    void KeyLogic();
    void HandleEvent(const sf::Event& event);
    void UpdateWorld();
    void DrawWorld();
    void RenderUi();
    void RenderSKeyOverlay();

    void ImguiMainMenuBar();
    void ImguiConsoleInputBox(float PADX, float PADY);
    void ImguiSpawnBrowser();
    void ImguiSpawnObjSettings();
    void ImguiSceneSettings();
};
