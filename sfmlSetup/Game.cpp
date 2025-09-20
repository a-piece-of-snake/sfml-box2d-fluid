#include "Game.h"

Game::Game()
    : window(sf::VideoMode({ 2400, 1350 }), "AWA", sf::Style::Default),
    worldView(sf::FloatRect({ 0.f, 0.f }, { static_cast<float>(width), static_cast<float>(height) })),
    uiView(window.getDefaultView()) {
}

Game::~Game() {}

void Game::Init() {
    std::wcout.imbue(std::locale("chs"));

    renderB2::init(&window);

    InitWindow();

    InitImGui();

    LoadResources();

    setConsoleTitle("Game Logs");
     
    fluid.init();

    InitScene();

    SUCCESS("Successfully initialized the game!");
}

void Game::Run() {
    while (window.isOpen()) {
        Update();
        Render();
    }
    return;
}

void Game::Destroy() {
    b2DestroyWorld(world.worldId);
    ImGui::SFML::Shutdown();
    return;
}

void Game::InitWindow() {

    window.setFramerateLimit(120);
    #ifdef _WIN32
        #pragma comment(lib, "dwmapi.lib")
        HWND hwnd = window.getNativeHandle();

        BOOL enabled = FALSE;
        HRESULT hr = DwmIsCompositionEnabled(&enabled);

        if (SUCCEEDED(hr) && enabled) {
            BOOL isDarkMode = TRUE;
            HRESULT hr = DwmSetWindowAttribute(
                hwnd,
                DWMWA_USE_IMMERSIVE_DARK_MODE,
                &isDarkMode,
                sizeof(isDarkMode)
            );

            if (FAILED(hr)) {
                DWORD color = 0x000000;
                DwmSetWindowAttribute(
                    hwnd,
                    DWMWA_CAPTION_COLOR,
                    &color,
                    sizeof(color)
                );
            }
        }
    #endif

    const sf::Image Icon("Assets\\Textures\\water2d.png");
    window.setIcon(Icon);

    sf::View view(sf::FloatRect({ 0.f, 0.f }, { (float)width, (float)height }));
    window.setView(view);

    return;
}

void Game::InitImGui() {
    if (!ImGui::SFML::Init(window)) ERROR("Failed to initialized Imgui!");

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.Fonts->Clear();

    io.Fonts->AddFontFromFileTTF(
        "Assets//Fonts//NanoDyongSong.ttf",
        18.0f,
        nullptr,
        io.Fonts->GetGlyphRangesChineseFull()
    );

    if (!ImGui::SFML::UpdateFontTexture()) ERROR("Failed to load font!");


    ImGui::GetStyle().ChildRounding = 12;
    ImGui::GetStyle().WindowRounding = 12;
    ImGui::GetStyle().FrameRounding = 12;
    ImGui::GetStyle().PopupRounding = 12;
    ImGui::GetStyle().ScrollbarRounding = 12;
    ImGui::GetStyle().TabRounding = 12;
    ImGui::GetStyle().TreeLinesRounding = 12;
    ImGui::GetStyle().GrabRounding = 12;

    ImGui::GetStyle().FrameBorderSize = 1;

    ImGui::GetStyle().SeparatorTextAlign.x = 0.5f;
    
    ImVec4* colors = ImGui::GetStyle().Colors; 

    colors[ImGuiCol_DockingPreview] = ImVec4(1.00f, 0.51f, 0.51f, 0.70f);
    colors[ImGuiCol_Text] = ImVec4(0.85f, 0.51f, 0.51f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.65f, 0.45f, 0.45f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.11f, 0.10f, 0.16f, 0.89f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.11f, 0.10f, 0.16f, 0.71f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.11f, 0.10f, 0.16f, 0.77f);
    colors[ImGuiCol_Border] = ImVec4(1.00f, 0.75f, 0.75f, 0.34f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.46f, 0.25f, 0.25f, 0.50f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(1.00f, 0.75f, 0.75f, 0.39f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(1.00f, 0.29f, 0.29f, 0.47f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.78f, 0.32f, 0.36f, 0.34f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.88f, 0.42f, 0.46f, 0.34f);
    colors[ImGuiCol_CheckMark] = ImVec4(1.00f, 0.62f, 0.85f, 1.00f);
    colors[ImGuiCol_TextLink] = ImVec4(0.93f, 0.50f, 0.96f, 1.00f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.93f, 0.70f, 0.70f, 0.40f);
    colors[ImGuiCol_TreeLines] = ImVec4(0.50f, 0.43f, 0.43f, 0.50f);
    colors[ImGuiCol_NavCursor] = ImVec4(0.98f, 0.33f, 0.26f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.84f, 0.40f, 0.40f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.79f, 0.33f, 0.33f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.98f, 0.26f, 0.26f, 0.40f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.98f, 0.26f, 0.26f, 0.80f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.94f, 0.20f, 0.20f, 0.89f);
    colors[ImGuiCol_Header] = ImVec4(0.98f, 0.26f, 0.26f, 0.31f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.98f, 0.26f, 0.26f, 0.80f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.96f, 0.37f, 0.37f, 1.00f);
    colors[ImGuiCol_Separator] = ImVec4(0.50f, 0.43f, 0.43f, 0.50f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.75f, 0.10f, 0.10f, 0.78f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.68f, 0.18f, 0.18f, 1.00f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.98f, 0.26f, 0.26f, 0.20f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.93f, 0.31f, 0.31f, 0.67f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.98f, 0.26f, 0.26f, 0.88f);
    colors[ImGuiCol_InputTextCursor] = ImVec4(0.72f, 0.34f, 0.34f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.98f, 0.26f, 0.26f, 0.80f);
    colors[ImGuiCol_Tab] = ImVec4(0.58f, 0.18f, 0.18f, 0.86f);
    colors[ImGuiCol_TabSelected] = ImVec4(0.68f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_TabSelectedOverline] = ImVec4(0.98f, 0.26f, 0.26f, 1.00f);
    colors[ImGuiCol_TabDimmedSelected] = ImVec4(0.42f, 0.14f, 0.14f, 1.00f);


    fileDialog.SetFlags(ImGuiFileBrowserFlags_EditPathString);
    fileDialog.SetTitle("title");
    fileDialog.SetTypeFilters({ ".h", ".cpp" });

    SUCCESS("Successfully initialized Imgui!");

    return;
}

void Game::InitScene() {

    world.timeStep = 0.1f;
    world.subStep = 4;


    b2WorldDef worldDef = b2DefaultWorldDef();

    worldDef.workerCount = world.workerCount;
    worldDef.enqueueTask = &CustomEnqueueTask; 
    worldDef.finishTask = &CustomFinishTask;

    

    worldDef.gravity = world.gravity;
    worldDef.contactHertz = world.contactHertz;
    worldDef.enableContinuous = false;
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
    mapBodyId.push_back(groundId);

    b2BodyDef groundBodyDef2 = b2DefaultBodyDef();
    groundBodyDef2.position.x = 400.f;
    groundBodyDef2.position.y = 1000.0f;
    groundBodyDef2.type = b2_staticBody;
    b2BodyId groundId2 = b2CreateBody(world.worldId, &groundBodyDef2);
    b2Polygon groundBox2 = b2MakeBox(1000.f, 10.0f);
    b2ShapeDef groundShapeDef2 = b2DefaultShapeDef();
    groundShapeDef2.density = 0.0f;
    b2CreatePolygonShape(groundId2, &groundShapeDef2, &groundBox2);
    mapBodyId.push_back(groundId2);

    b2BodyDef groundBodyDef3 = b2DefaultBodyDef();
    groundBodyDef3.position.x = 400.f;
    groundBodyDef3.position.y = -1000.f;
    groundBodyDef3.type = b2_staticBody;
    b2BodyId groundId3 = b2CreateBody(world.worldId, &groundBodyDef3);
    b2Polygon groundBox3 = b2MakeBox(1000.f, 10.f);
    b2ShapeDef groundShapeDef3 = b2DefaultShapeDef();
    groundShapeDef3.density = 0.0f;
    b2CreatePolygonShape(groundId3, &groundShapeDef3, &groundBox3);
    mapBodyId.push_back(groundId3);
    /*
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
    */
	SUCCESS("Successfully initialized the scene!");

    return;
   }

void Game::LoadResources() {
    BackGroundT.loadFromFile("Assets\\Textures\\BackGround.png");
    BackGroundT.setRepeated(true);
    Snake.loadFromFile("Assets\\Textures\\snake.png");
    smoothFunctionT.loadFromFile("Assets\\Textures\\SmoothFunction.png");

    resources.shaderlist.addShader("Star", "", "", "Assets\\Shaders\\star.frag");

    {
        GameObjects::SpawnableObject box;
        box.name = "Box";
        box.type = "Solid";
        box.describe = "An ordinary box.";
        box.icon = Snake;
        box.onSpawned = [this](b2Vec2 pos, float size, float friction, float restitution) -> bool {

            this->createSquare(pos.x, pos.y, 2.0f, friction, restitution, 15.0f * size);
            
            return true;
            };
	    spawnBrowser.addObject(box);
    }
    {
        GameObjects::SpawnableObject bigBox;
        bigBox.name = "Big Box";
        bigBox.type = "Solid";
        bigBox.describe = "An bigggg box.";
        bigBox.icon = BackGroundT;
        bigBox.onSpawned = [this](b2Vec2 pos, float size, float friction, float restitution) -> bool {

            this->createSquare(pos.x, pos.y, 0.01f, friction, restitution, 75.0f * size);

            return true;
            };
        spawnBrowser.addObject(bigBox);
    }
    {
        GameObjects::SpawnableObject testobj2;
        testobj2.name = "Circle";
        testobj2.type = "Solid";
        testobj2.describe = "An ordinary circle.";
        testobj2.icon = smoothFunctionT;
        testobj2.onSpawned = [this](b2Vec2 pos, float size, float friction, float restitution) -> bool {

            this->createCircle(pos.x, pos.y, 2.0f, friction, restitution, 8.0f * size);

            return true;
            };
        spawnBrowser.addObject(testobj2);
    }
    {
        GameObjects::SpawnableObject cup;
        cup.name = "Cup";
        cup.type = "Solid";
        cup.describe = "An ordinary cup.";
        cup.icon = Snake;
        cup.onSpawned = [this](b2Vec2 pos, float size, float friction, float restitution) -> bool {

            this->createCup(pos.x, pos.y, friction, restitution, 200.f * size);
            return true;
            };
        spawnBrowser.addObject(cup);
    }
    {
        GameObjects::SpawnableObject cross;
        cross.name = "Cross";
        cross.type = "Solid";
        cross.describe = "An ordinary cross.";
        cross.icon = Snake;
        cross.onSpawned = [this](b2Vec2 pos, float size, float friction, float restitution) -> bool {

            this->createCross(pos.x, pos.y, friction, restitution, 200.f * size);
            return true;
            };
        spawnBrowser.addObject(cross);
    }

    SUCCESS("Successfully loaded resouces!");

    return;
}

void Game::Update() {
    mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
    worldPos = MathUtils::toB2Position(mousePos.x, mousePos.y, worldView);
    elapsed = msclock.restart();
    elapsedTime = elapsed.asMilliseconds();

    while (const std::optional event = window.pollEvent()) {
        ImGui::SFML::ProcessEvent(window, *event);
        if (event->is<sf::Event::Closed>()) {
            window.close();
        }
        if (window.hasFocus())
            HandleEvent(*event);
    }
    ImGuiRelated();
    if(window.hasFocus())
        KeyLogic();
    UpdateWorld();

    return;
}

void Game::Render() {
    window.clear(renderB2::DefaultColors::ClearFill);

    window.setView(worldView);

    sf::Vector2i offset = (sf::Vector2i)window.mapPixelToCoords({ 0, 0 });

    window.setView(uiView);

    //sf::RectangleShape BackGround({ (float)width - 350 * 2, (float)height - 125.f * 2 });
    sf::RectangleShape BackGround({ (float)width, (float)height });
    //BackGround.move({ 350.f, 200.f });
    BackGround.setOutlineThickness(3);
    BackGround.setOutlineColor(renderB2::DefaultColors::BackGroundOutline);
    BackGround.setTexture(&BackGroundT);
    BackGround.setTextureRect(sf::IntRect{
    {
         (int)-(BackGround.getSize().x / 3) + (int)((float)offset.x / 1.5),
         (int)-(BackGround.getSize().y / 3) + (int)((float)offset.y / 1.5)
    },
    {
         (int)(BackGround.getSize().x / 1.5),
         (int)(BackGround.getSize().y / 1.5)
    }
        });
    window.draw(BackGround);
    window.setView(worldView);
    DrawWorld();
    if (selection.isSelecting) {
        sf::RectangleShape selectionBox;
        sf::Vector2f size = MathUtils::getSFpos(selection.currentPos.x, selection.currentPos.y) - MathUtils::getSFpos(selection.startPos.x, selection.startPos.y);
        selectionBox.setTexture(&Snake);
        selectionBox.setSize(sf::Vector2f(std::abs(size.x), std::abs(size.y)));
        b2Vec2 aaa = { std::max(selection.startPos.x, selection.currentPos.x),
            std::min(selection.startPos.y, selection.currentPos.y) };
        selectionBox.setPosition(MathUtils::getSFpos(aaa.x, aaa.y));
        selectionBox.setFillColor(sf::Color(100, 100, 255, 50));
        selectionBox.setOutlineColor(sf::Color::Blue);
        selectionBox.setOutlineThickness(2);
        window.draw(selectionBox);
    }
    
    window.setView(uiView);
    /* {
        sf::RectangleShape backgroundBorder = BackGround;
        backgroundBorder.move({ -BackGround.getOutlineThickness(), -BackGround.getOutlineThickness() });
        backgroundBorder.setSize({ BackGround.getSize().x + BackGround.getOutlineThickness() * 2, BackGround.getSize().y + BackGround.getOutlineThickness() * 2 });
        backgroundBorder.setFillColor(sf::Color::Transparent);
        backgroundBorder.setOutlineColor(renderB2::DefaultColors::ClearFill);
        backgroundBorder.setOutlineThickness(1000.0f);
        window.draw(backgroundBorder);
    }*/

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) {
        sf::CircleShape shape(Dragdef.radius, 32);
        shape.setOutlineThickness(2);
        shape.setOutlineColor(sf::Color::Blue);
        shape.setFillColor(sf::Color::Transparent);
        shape.setOrigin({ Dragdef.radius, Dragdef.radius });
        shape.setPosition(sf::Vector2f(mousePos.x, mousePos.y));
        window.draw(shape);
    }


    RenderUi();
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) {
        RenderSKeyOverlay();
    }
    ImGui::SFML::Render(window);
    window.display();
    lastmousePos = mousePos;
    tickCount++;

    return;
}

void Game::RenderSKeyOverlay() {
    {
        sf::CircleShape starbackground(100, 32);
        starbackground.setOrigin({ 100, 100 });

        sf::Shader* starShaderPtr = resources.shaderlist.getShader("Star");

        if (starShaderPtr) {
            sf::Shader& starShader = *starShaderPtr;
            starbackground.move({ (float)mousePos.x, (float)mousePos.y });
            starShader.setUniform("iResolution", sf::Vector2f(window.getSize()));
            starShader.setUniform("iTime", world.clock.getElapsedTime().asSeconds());
            window.draw(starbackground, &starShader);
        }
        else {
            ERROR("Shader not found!");
			return;
        }
    }
    /*
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
    */
}

void Game::ImGuiRelated() {


    ImGui::SFML::Update(window, deltaClock.restart());

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | 
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    
    
    ImGui::Begin("DockSpace", nullptr, window_flags);
    ImGui::PopStyleVar(2);

    ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_NoDockingInCentralNode);
    

    ImGui::End();

    //ImGui::ShowDemoWindow();
    //ImGui::ShowStyleEditor();

    if (ImGui::IsKeyPressed(ImGuiKey_GraveAccent)) {
		showConsole = !showConsole;
    }
	consoleAnimation = showConsole ? consoleAnimation + 1.f : consoleAnimation - 1.f;
    consoleAnimation = std::clamp(consoleAnimation, -40.f, 10.f);
    ImguiConsoleInputBox(10.f, consoleAnimation);

    ImguiSpawnBrowser();
    ImguiSpawnObjSettings(); 
    ImguiSceneSettings();

    ImguiMainMenuBar();

    if (ImGui::Begin("Fluid Parameters")) {

	    ImGui::Text("Particle count: %d", particleCount);
        ImGui::Text("Ctrl + Click to edit it");

        ImGui::SliderFloat("surface##Slider", &fluid.Config.FORCE_SURFACE, 0.f, 5000.f, nullptr, ImGuiSliderFlags_None);
            ImGui::SameLine();
		    if (ImGui::ArrowButton("##surface left", ImGuiDir_Left)) fluid.Config.FORCE_SURFACE -= 10.f;
            ImGui::SameLine();
		    if (ImGui::ArrowButton("##surface right", ImGuiDir_Right)) fluid.Config.FORCE_SURFACE += 10.f;
            ImGui::SameLine();
			if (ImGui::Button("reset##surface")) fluid.Config.FORCE_SURFACE = 75.f;
            /*
        ImGui::SliderFloat("viscosity##Slider", &fluid.Config.SHEAR_VISCOSITY, 0.f, 110.f, nullptr, ImGuiSliderFlags_None);
            ImGui::SameLine();
            if (ImGui::ArrowButton("##viscosity left", ImGuiDir_Left)) fluid.Config.SHEAR_VISCOSITY -= 1.f;
            ImGui::SameLine();
            if (ImGui::ArrowButton("##viscosity right", ImGuiDir_Right)) fluid.Config.SHEAR_VISCOSITY += 1.f;
            ImGui::SameLine();
            if (ImGui::Button("reset##viscosity")) fluid.Config.SHEAR_VISCOSITY = 20.f;

		fluid.Config.VISCOSITY = fluid.Config.SHEAR_VISCOSITY * 0.4f;
        fluid.Config.VISCOSITY_LEAVE = fluid.Config.SHEAR_VISCOSITY * 0.02f;
        */

         ImGui::SliderFloat("shear viscosity##Slider", &fluid.Config.SHEAR_VISCOSITY, 0.f, 150.f, nullptr, ImGuiSliderFlags_None);
            ImGui::SameLine();
            if (ImGui::ArrowButton("##shear viscosity left", ImGuiDir_Left)) fluid.Config.SHEAR_VISCOSITY -= 1.f;
            ImGui::SameLine();
            if (ImGui::ArrowButton("##shear viscosity right", ImGuiDir_Right)) fluid.Config.SHEAR_VISCOSITY += 1.f;
            ImGui::SameLine();
            if (ImGui::Button("reset##shear viscosity")) fluid.Config.SHEAR_VISCOSITY = 20.f;

         ImGui::SliderFloat("viscosity##Slider", &fluid.Config.VISCOSITY, 0.f, 60.f, nullptr, ImGuiSliderFlags_None);
            ImGui::SameLine();
            if (ImGui::ArrowButton("##viscosity left", ImGuiDir_Left)) fluid.Config.VISCOSITY -= 1.f;
            ImGui::SameLine();
            if (ImGui::ArrowButton("##viscosity right", ImGuiDir_Right)) fluid.Config.VISCOSITY += 1.f;
            ImGui::SameLine();
            if (ImGui::Button("reset##viscosity")) fluid.Config.VISCOSITY = 8.f;

         ImGui::SliderFloat("viscosity leave##Slider", &fluid.Config.VISCOSITY_LEAVE, 0.f, 2.5f, nullptr, ImGuiSliderFlags_None);
            ImGui::SameLine();
            if (ImGui::ArrowButton("##viscosity leave left", ImGuiDir_Left)) fluid.Config.VISCOSITY_LEAVE -= 0.1f;
            ImGui::SameLine();
            if (ImGui::ArrowButton("##viscosity leave right", ImGuiDir_Right)) fluid.Config.VISCOSITY_LEAVE += 0.1f;
            ImGui::SameLine();
            if (ImGui::Button("reset##viscosity leave")) fluid.Config.VISCOSITY_LEAVE = 0.8f;

    }ImGui::End();

    if (ImGui::Begin("dummy window"))
    {
        // open file dialog when user clicks this button
        if (ImGui::Button("open file dialog"))
            fileDialog.Open();

    }ImGui::End();
    
    fileDialog.Display();

    if (fileDialog.HasSelected())
    {
		DEBUG("Selected filename: %s", fileDialog.GetSelected().string().c_str());
        fileDialog.ClearSelected();
    }
    return;
}

void Game::HandleEvent(const sf::Event& event) {
    if (const auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mousePressed->button == sf::Mouse::Button::Left) {

            if (B2_IS_NON_NULL(world.mouseJointId)) {
                b2DestroyJoint(world.mouseJointId);
                world.mouseJointId = {};
            }
            b2AABB box;
            b2Vec2 d = { 0.001f, 0.001f };
            b2Vec2 p = worldPos;
            box.lowerBound = b2Sub(p, d);
            box.upperBound = b2Add(p, d);

            // Query the world for overlapping shapes.
            QueryContext queryContext = { p, b2_nullBodyId };
            b2World_OverlapAABB(world.worldId, box, b2DefaultQueryFilter(), QueryCallback, &queryContext);

            if (B2_IS_NON_NULL(queryContext.bodyId))
            {
                b2Body_SetAwake(queryContext.bodyId, true);
                b2BodyDef bodyDef = b2DefaultBodyDef();
                world.groundBodyId = b2CreateBody(world.worldId, &bodyDef);

                b2MouseJointDef mouseDef = b2DefaultMouseJointDef();
                mouseDef.bodyIdA = world.groundBodyId;
                mouseDef.bodyIdB = queryContext.bodyId;
                mouseDef.target = p;
                mouseDef.hertz = 2.5f;
                mouseDef.dampingRatio = 0.7f;
                mouseDef.maxForce = 100.0f * b2Body_GetMass(queryContext.bodyId);
                world.mouseJointId = b2CreateMouseJoint(world.worldId, &mouseDef);

            }
        }
        if (mousePressed->button == sf::Mouse::Button::Right && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LAlt)) {

            b2AABB box;
            b2Vec2 d = { 0.001f, 0.001f };
            b2Vec2 p = worldPos;
            box.lowerBound = b2Sub(p, d);
            box.upperBound = b2Add(p, d);

            // Query the world for overlapping shapes.
            QueryContext queryContext = { p, b2_nullBodyId };
            b2World_OverlapAABB(world.worldId, box, b2DefaultQueryFilter(), QueryCallback, &queryContext);

            if (B2_IS_NON_NULL(queryContext.bodyId))
            {
                b2Body_SetAwake(queryContext.bodyId, true);
                if (b2Body_GetType(queryContext.bodyId) == b2_staticBody)
				    b2Body_SetType(queryContext.bodyId, b2_dynamicBody);
                else 
					b2Body_SetType(queryContext.bodyId, b2_staticBody);
            }
        }
    }
    if (const auto* mouseReleased = event.getIf<sf::Event::MouseButtonReleased>()) {
        if (mouseReleased->button == sf::Mouse::Button::Left && selection.isSelecting) {

            b2Vec2 start = selection.startPos;
            b2Vec2 end = selection.currentPos;

            float minX = std::min(start.x, end.x);
            float maxX = std::max(start.x, end.x);
            float minY = std::min(start.y, end.y);
            float maxY = std::max(start.y, end.y);
            float Xlen = std::abs(maxX - minX);
            float Ylen = std::abs(maxY - minY);
            const float gridSize = 135;//water
            //const float gridSize = 75;
            float stepX = (maxX - minX) / (Xlen * gridSize / window.getSize().x);
            float stepY = (maxY - minY) / (Ylen * gridSize / window.getSize().y);
            DEBUG("Selection X: %f Y: %f", (Xlen * gridSize / window.getSize().x), (Ylen * gridSize / window.getSize().y));
            for (int i = 0; i < (Xlen * gridSize / window.getSize().x); ++i) {
                for (int j = 0; j < (Ylen * gridSize / window.getSize().y); ++j) {
                    float x = minX + i * stepX + stepX / 2;
                    float y = minY + j * stepY + stepY / 2;
                    //createSquare(x, y, 0.5, 0.3, 0.1, 5);
                    //fluid.CreateParticle(world, -0.1, 4, x, y, 1.0f, 0.f, 0.1f); // smoke
                    fluid.CreateParticle(world, 4, x, y, 2.5f, selectedObjectFriction, selectedObjectRestitution, sf::Color::Cyan); // water
                    //fluid.CreateParticle(world, 1, 4, x, y, 2.5f, 2.f, 0.1f, sf::Color::Cyan); // sand
                    particleCount++;
                }
            }
            selection.isSelecting = false;
        }
    }
    /*
    if (const auto* MouseWheelScrolled = event.getIf<sf::Event::MouseWheelScrolled>()) {
        float zoomFactor = (MouseWheelScrolled->delta > 0) ? 0.95f : 1.05f;
        worldView.zoom(zoomFactor);
    }*/
    
    if (const auto* mouseMoved = event.getIf<sf::Event::MouseMoved>()) {
        if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Middle)) {
            worldView.move({ (int)lastmousePos.x - (float)mousePos.x, (int)lastmousePos.y - (float)mousePos.y });
            camX = worldView.getCenter().x - worldView.getSize().x / 2;
            camY = worldView.getCenter().y - worldView.getSize().y / 2;
        }
        if (selection.isSelecting) {
            selection.currentPos = MathUtils::toB2Position(window.mapPixelToCoords(sf::Mouse::getPosition(window)).x, window.mapPixelToCoords(sf::Mouse::getPosition(window)).y, worldView);
        }
    }
    if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>())
    {
        if (keyPressed->scancode == sf::Keyboard::Scan::Space)
        {
            PauseWorld = !PauseWorld;
        }
        /*
        if (keyPressed->scancode == sf::Keyboard::Scan::F)
        {
            fluid.freeze();
        }
        if (keyPressed->scancode == sf::Keyboard::Scan::G)
        {
            fluid.unfreeze();
        }
        */
        if (keyPressed->scancode == sf::Keyboard::Scan::P)
        {
            renderParticle = (renderParticle + 1) % 5;
        }

        if (keyPressed->scancode == sf::Keyboard::Scan::R) {
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
            INFO("All deleted");
        }

        if (!(ImGui::IsAnyItemHovered())) {
            if (keyPressed->scancode == sf::Keyboard::Scan::Num1) {
                if (selectedObject[0] != nullptr) {
                    if (!selectedObject[0]->spawn(worldPos, selectedObjectSize, selectedObjectFriction, selectedObjectRestitution)) {
                        ERROR("Failed to spawn object!");
                    }
                }
            }
            if (keyPressed->scancode == sf::Keyboard::Scan::Num2) {
                if (selectedObject[1] != nullptr) {
                    if (!selectedObject[1]->spawn(worldPos, selectedObjectSize, selectedObjectFriction, selectedObjectRestitution)) {
                        ERROR("Failed to spawn object!");
                    }
                }
            }
            if (keyPressed->scancode == sf::Keyboard::Scan::Num3) {
                if (selectedObject[2] != nullptr) {
                    if (!selectedObject[2]->spawn(worldPos, selectedObjectSize, selectedObjectFriction, selectedObjectRestitution)) {
                        ERROR("Failed to spawn object!");
                    }
                }
            }
            if (keyPressed->scancode == sf::Keyboard::Scan::Num4) {
                if (selectedObject[3] != nullptr) {
                    if (!selectedObject[3]->spawn(worldPos, selectedObjectSize, selectedObjectFriction, selectedObjectRestitution)) {
                        ERROR("Failed to spawn object!");
                    }
                }
            }
            if (keyPressed->scancode == sf::Keyboard::Scan::Num5) {
                if (selectedObject[4] != nullptr) {
                    if (!selectedObject[4]->spawn(worldPos, selectedObjectSize, selectedObjectFriction, selectedObjectRestitution)) {
                        ERROR("Failed to spawn object!");
                    }
                }
            }
            if (keyPressed->scancode == sf::Keyboard::Scan::Num6) {
                if (selectedObject[5] != nullptr) {
                    if (!selectedObject[5]->spawn(worldPos, selectedObjectSize, selectedObjectFriction, selectedObjectRestitution)) {
                        ERROR("Failed to spawn object!");
                    }
                }
            }
            if (keyPressed->scancode == sf::Keyboard::Scan::Num7) {
                if (selectedObject[6] != nullptr) {
                    if (!selectedObject[6]->spawn(worldPos, selectedObjectSize, selectedObjectFriction, selectedObjectRestitution)) {
                        ERROR("Failed to spawn object!");
                    }
                }
            }
            if (keyPressed->scancode == sf::Keyboard::Scan::Num8) {
                if (selectedObject[7] != nullptr) {
                    if (!selectedObject[7]->spawn(worldPos, selectedObjectSize, selectedObjectFriction, selectedObjectRestitution)) {
                        ERROR("Failed to spawn object!");
                    }
                }
            }
            if (keyPressed->scancode == sf::Keyboard::Scan::Num9) {
                if (selectedObject[8] != nullptr) {
                    if (!selectedObject[8]->spawn(worldPos, selectedObjectSize, selectedObjectFriction, selectedObjectRestitution)) {
                        ERROR("Failed to spawn object!");
                    }
                }
            }
            if (keyPressed->scancode == sf::Keyboard::Scan::Num0) {
                if (selectedObject[9] != nullptr) {
                    if (!selectedObject[9]->spawn(worldPos, selectedObjectSize, selectedObjectFriction, selectedObjectRestitution)) {
                        ERROR("Failed to spawn object!");
                    }
                }
            }
        }

    }
    return;
}

void Game::UpdateWorld() {
	b2World_SetGravity(world.worldId,world.gravity);
	b2World_SetContactTuning(world.worldId, world.contactHertz, 0.5f, 10.f);
    b2World_SetJointTuning(world.worldId, world.contactHertz, 0.5f);
    if (!PauseWorld) {
        fluid.UpdateData(world);
        fluid.ComputeParticleForces(world.timeStep);
        b2World_Step(world.worldId, world.timeStep, world.subStep);
    }
    return;
}

void Game::DrawWorld() {
    renderB2::RenderSettings rendersettings;
    rendersettings.OutlineThickness = 1;
    rendersettings.OutlineColor = sf::Color::White;
    rendersettings.FillColor = renderB2::DefaultColors::B2BodyFill;
    rendersettings.verticecount = 4;

    renderB2::ScreenSettings screensettings;
    screensettings.camX = camX;
    screensettings.camY = camY;
    screensettings.width = width;
    screensettings.height = height;
    for (auto& mapbodyId : mapBodyId) {
        int mapbodyShapeCount = b2Body_GetShapeCount(mapbodyId);
        b2ShapeId mapbodyShape;
        b2Body_GetShapes(mapbodyId, &mapbodyShape, mapbodyShapeCount);
        renderB2::renderb2Polygon(&window, rendersettings, b2Shape_GetPolygon(mapbodyShape), mapbodyId, screensettings);
    }
    //renderB2::renderSoul(&window, playerCircle, playerId, screensettings);

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
        renderB2::renderwatershader(&window, rendersettings, fluid, screensettings);
        break;
    case 2:
        renderB2::rendersimpleparticle(&window, rendersettings, fluid, screensettings);
        break;
    case 3:
        renderB2::renderparticletest(&window, rendersettings, fluid, screensettings);
        break;
    case 4:
        renderB2::rendersandparticle(&window, rendersettings, fluid, screensettings);
        break;
    default:
		ERROR("RenderParticle mode error!");
    }

    /*
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
    */
    rendersettings.verticecount = 4;

    if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
        if (B2_IS_NON_NULL(world.mouseJointId)) {
            b2BodyId bodyBID = b2Joint_GetBodyB(world.mouseJointId);
            b2Vec2 posA = b2MouseJoint_GetTarget(world.mouseJointId);
            b2Vec2 posBL = b2Joint_GetLocalAnchorB(world.mouseJointId);
            posBL = b2RotateVector(b2Body_GetRotation(bodyBID), posBL);
            b2Vec2 posB = b2Body_GetPosition(bodyBID) + posBL;
            sf::Vertex line[2] = {
                sf::Vertex{ MathUtils::getSFpos(posA.x, posA.y), sf::Color::White},
                sf::Vertex{ MathUtils::getSFpos(posB.x, posB.y), sf::Color::Red }
            };
            window.draw(line, 2, sf::PrimitiveType::LineStrip);
        }
    }

    return;
}

void Game::KeyLogic() {


    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift)) {
        selection.isSelecting = true;
        selection.startPos = MathUtils::toB2Position(window.mapPixelToCoords(sf::Mouse::getPosition(window)).x, window.mapPixelToCoords(sf::Mouse::getPosition(window)).y, worldView);
        selection.currentPos = selection.startPos;
    }

    Dragdef = b2DefaultExplosionDef();

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) {
        Drag = true;
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        Dragdef.position = b2Vec2{ worldPos.x, worldPos.y };
        Dragdef.radius = 100.0f;
        Dragdef.falloff = 5.f;
        Dragdef.impulsePerLength = -70.0f;
        b2World_Explode(world.worldId, &Dragdef);
    }


    if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
        if (B2_IS_NON_NULL(world.mouseJointId)) {

            if (!B2_IS_NON_NULL(b2Joint_GetBodyB(world.mouseJointId))) {
                b2DestroyJoint(world.mouseJointId);
                world.mouseJointId = {};
            }
            else {
                b2Body_SetAwake(b2Joint_GetBodyB(world.mouseJointId), true);
                b2MouseJoint_SetTarget(world.mouseJointId, worldPos);
            }
        }
    }
    else {
        if (B2_IS_NON_NULL(world.mouseJointId)) {
            b2DestroyJoint(world.mouseJointId);
            world.mouseJointId = {};
        }
    }/*
    if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Right)) {
        Drag = true;
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        Dragdef.position = b2Vec2{ worldPos.x, worldPos.y };
        Dragdef.radius = 50.0f;
        Dragdef.falloff = 0.f;
        Dragdef.impulsePerLength = 100.0f;
        b2World_Explode(world.worldId, &Dragdef);
    }*/
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
    return;
}

void Game::ImguiMainMenuBar() {

    if (ImGui::BeginMainMenuBar()) {

        if (ImGui::BeginMenu("File")) {
            ImGui::Button("aaa");
        ImGui::EndMenu();}

        if (ImGui::BeginMenu("Edit")) {
            ImGui::Button("aaa");
        ImGui::EndMenu();}

    }ImGui::EndMainMenuBar();
    return;
}

void Game::ImguiConsoleInputBox(float PADX, float PADY) {

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoBringToFrontOnFocus;
    
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 work_pos = viewport->WorkPos;
    ImVec2 work_size = viewport->WorkSize;
    ImVec2 window_pos, window_pos_pivot;
    window_pos.x = (work_pos.x + PADX);
    window_pos.y = (work_pos.y + PADY);
    window_pos_pivot.x = 0.0f;
    window_pos_pivot.y = 0.0f;
    ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
    window_flags |= ImGuiWindowFlags_NoMove;

    if (ImGui::Begin("##user command", nullptr , window_flags)) {

        char inputBuffer[1024] = "";
        if (ImGui::InputText("Command", inputBuffer, sizeof(inputBuffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
            DEBUG("User command: %s", userCommend.c_str());
        }
        else {
            if (userCommend != std::string(inputBuffer)) {
            }
        }
        userCommend = std::string(inputBuffer);

    ImGui::End();}

    return;
}

void Game::ImguiSpawnBrowser() {
    if (ImGui::Begin("Spawnable Objects Browser")) {
        if (ImGui::InputText("Search", spawnBrowser.searchBuffer, sizeof(spawnBrowser.searchBuffer))) {
            spawnBrowser.search = std::string(spawnBrowser.searchBuffer);
        }
        ImGui::NewLine();
        ImGui::Separator();

        ImGui::Text("Objects :");
        ImGui::NewLine();

        ImGui::BeginGroup();

        float total_width = ImGui::GetContentRegionAvail().x;
        float button_width = spawnBrowser.iconSize + ImGui::GetStyle().ItemSpacing.x * 2;
        int buttons_per_row = (total_width + ImGui::GetStyle().ItemSpacing.x) / button_width;

        if (buttons_per_row == 0) {
            buttons_per_row = 1; 
        }

        int index = 0;
        for (auto& obj : spawnBrowser.objects) {
            if (obj.name.find( spawnBrowser.search) == std::string::npos) continue;
            if (ImGui::ImageButton((obj.name + std::to_string(index)).c_str(), obj.icon, { spawnBrowser.iconSize, spawnBrowser.iconSize }, renderB2::DefaultColors::Button)) {
                //selectedObject[index] = &obj;
            }

            if (ImGui::IsItemHovered()) {
                if (ImGui::BeginItemTooltip()) {

                    ImGui::Text(obj.name.c_str());
                    ImGui::Separator();
                    ImGui::Text(obj.describe.c_str());

                    ImGui::EndTooltip();
                }
                if (ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_1)) {
                    selectedObject[0] = &obj;
                }
                else if (ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_2)) {
                    selectedObject[1] = &obj;
                }
                else if (ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_3)) {
                    selectedObject[2] = &obj;
                }
                else if (ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_4)) {
                    selectedObject[3] = &obj;
                }
                else if (ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_5)) {
                    selectedObject[4] = &obj;
                }
                else if (ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_6)) {
                    selectedObject[5] = &obj;
                }
                else if (ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_7)) {
                    selectedObject[6] = &obj;
                }
                else if (ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_8)) {
                    selectedObject[7] = &obj;
                }
                else if (ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_9)) {
                    selectedObject[8] = &obj;
                }
                else if (ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_0)) {
                    selectedObject[9] = &obj;
                }
            }
            /*
            if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
                INFO("right");
            }
            */
            if ((index + 1) % buttons_per_row != 0 && (index + 1) < spawnBrowser.objects.size()) {
                ImGui::SameLine();
            }
            index++;
        }

        ImGui::EndGroup();

    }ImGui::End();

    return;
}

void Game::ImguiSpawnObjSettings() {
    if (ImGui::Begin("Spawnable Objects Settings")) {
         ImGui::SliderFloat("Size##Slider", &selectedObjectSize, 0.f, 10.f, nullptr, ImGuiSliderFlags_None);
            ImGui::SameLine();
            if (ImGui::ArrowButton("##Size left", ImGuiDir_Left)) selectedObjectSize -= 1.f;
            ImGui::SameLine();
            if (ImGui::ArrowButton("##Size right", ImGuiDir_Right)) selectedObjectSize += 1.f;
            ImGui::SameLine();
            if (ImGui::Button("reset##Size")) selectedObjectSize = 1.f;
         ImGui::SliderFloat("Friction##Slider", &selectedObjectFriction, 0.f, 1.f, nullptr, ImGuiSliderFlags_None);
            ImGui::SameLine();
            if (ImGui::ArrowButton("##Friction left", ImGuiDir_Left)) selectedObjectFriction -= 0.05f;
            ImGui::SameLine();
            if (ImGui::ArrowButton("##Friction right", ImGuiDir_Right)) selectedObjectFriction += 0.05f;
            ImGui::SameLine();
            if (ImGui::Button("reset##Friction")) selectedObjectFriction = 0.8f;
         ImGui::SliderFloat("Restitution##Slider", &selectedObjectRestitution, 0.f, 1.f, nullptr, ImGuiSliderFlags_None);
            ImGui::SameLine();
            if (ImGui::ArrowButton("##Restitution left", ImGuiDir_Left)) selectedObjectRestitution -= 0.05f;
            ImGui::SameLine();
            if (ImGui::ArrowButton("##Restitution right", ImGuiDir_Right)) selectedObjectRestitution += 0.05f;
            ImGui::SameLine();
            if (ImGui::Button("reset##Restitution")) selectedObjectRestitution = 0.2f;
    }ImGui::End();

    return;
}
void Game::ImguiSceneSettings() {
    if (ImGui::Begin("Scene Settings")) {
        ImGui::SliderFloat("Gravity X##Slider", &world.gravity.x, 10.f, -10.f, nullptr, ImGuiSliderFlags_None);
            ImGui::SameLine();
            if (ImGui::ArrowButton("##Gravity X left", ImGuiDir_Left)) world.gravity.x -= 0.5f;
            ImGui::SameLine();
            if (ImGui::ArrowButton("##Gravity X right", ImGuiDir_Right)) world.gravity.x += 0.5f;
            ImGui::SameLine();
            if (ImGui::Button("reset##Gravity X")) world.gravity.x = -2.5f;
        ImGui::SliderFloat("Gravity Y##Slider", &world.gravity.y, 10.f, -10.f, nullptr, ImGuiSliderFlags_None);
            ImGui::SameLine();
            if (ImGui::ArrowButton("##Gravity Y left", ImGuiDir_Left)) world.gravity.y -= 0.5f;
            ImGui::SameLine();
            if (ImGui::ArrowButton("##Gravity Y right", ImGuiDir_Right)) world.gravity.y += 0.5f;
            ImGui::SameLine();
            if (ImGui::Button("reset##Gravity Y")) world.gravity.y = 0.f;
        ImGui::SliderFloat("Contact Hertz##Slider", &world.contactHertz, 0.f, 128.f, nullptr, ImGuiSliderFlags_None);
            ImGui::SameLine();
            if (ImGui::ArrowButton("##Contact Hertz left", ImGuiDir_Left)) world.contactHertz -= 0.5f;
            ImGui::SameLine();
            if (ImGui::ArrowButton("##Contact Hertz right", ImGuiDir_Right)) world.contactHertz += 0.5f;
            ImGui::SameLine();
            if (ImGui::Button("reset##Contact Hertz")) world.contactHertz = 60.f;
    }ImGui::End();
	return;
}
void Game::RenderUi() {
    {//左边ui
        //sf::RectangleShape leftUIshape({ 350.f - 8.f,  (float)height - 225.f });
        sf::RectangleShape leftUIshape({ 350.f - 20.f,  (float)height - 50.f });
        leftUIshape.setFillColor(sf::Color::Transparent);
        //renderB2::MyUIObject* leftUI = new renderB2::MyUIObject(&leftUIshape, sf::Vector2f(8.f, 0.f + 200.f));
        renderB2::MyUIObject* leftUI = new renderB2::MyUIObject(&leftUIshape, sf::Vector2f(20.f, 0.f + 50.f));
        leftUI->lineSpacing = 5.f;
        //{
            sf::ConvexShape ticktime = renderB2::getRectangleMinusCorners(330.f, 115.f, 11.5f);
            ticktime.setFillColor(sf::Color::Transparent);
            ticktime.setOutlineThickness(3.f);
            ticktime.setFillColor(renderB2::DefaultColors::B2BodyFill2);
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
            if (fpsClock.getElapsedTime().asSeconds() >= 1.0f) {
                fps = tickCount / fpsClock.getElapsedTime().asSeconds();
                tickCount = 0;
                fpsClock.restart();
            }
            sf::ConvexShape tpsshape = renderB2::getRectangleMinusCorners(330.f, 115.f, 11.5f);
            tpsshape.setFillColor(sf::Color::Transparent);
            tpsshape.setOutlineThickness(3.f);
            tpsshape.setFillColor(renderB2::DefaultColors::B2BodyFill2);
            renderB2::MyUIObject* root2 = new renderB2::MyUIObject(&tpsshape, { 0, 0 }, 0);
            sf::Text awa2(renderB2::getDefaultFontAddress());
            awa2.setString("FPS\n" + std::to_string((int)fps));
            awa2.setStyle(sf::Text::Bold);
            awa2.setCharacterSize(30.f);
            if (fps <= 60) {
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
            if (fpsClock.getElapsedTime().asSeconds() >= 1.0f) {
                fps = tickCount / fpsClock.getElapsedTime().asSeconds();
                tickCount = 0;
                fpsClock.restart();
            }
            sf::ConvexShape particlecount = renderB2::getRectangleMinusCorners(330.f, 115.f, 11.5f);
            particlecount.setFillColor(sf::Color::Transparent);
            particlecount.setOutlineColor(sf::Color::White);
            particlecount.setOutlineThickness(3.f);
            particlecount.setFillColor(renderB2::DefaultColors::B2BodyFill2);
            renderB2::MyUIObject* root3 = new renderB2::MyUIObject(&particlecount, { 0, 0 }, 0);
            sf::Text awa3(renderB2::getDefaultFontAddress());
            awa3.setString("Particle Count\n     " + std::to_string(particleCount));
            awa3.setStyle(sf::Text::Bold);
            awa3.setCharacterSize(30.f);
            awa3.setFillColor(sf::Color::White);
            awa3.setOrigin({ awa3.getLocalBounds().size.x / 2, awa3.getLocalBounds().size.y / 2 });
            renderB2::MyUIObject* child3 = new renderB2::MyUIObject(&awa3, sf::Vector2f(50.f, 0.f));
            child3->localPosition = { particlecount.getLocalBounds().size.x / 2, particlecount.getLocalBounds().size.y / 2 };
            root3->addChild(child3);
            //root->localPosition = { 8.f, 277.f };
            //root->draw(&window);
            leftUI->addChild(root3);
        leftUI->draw(&window, true, true);
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
    return;
}

bool Game::QueryCallback(b2ShapeId shapeId, void* context) {
    QueryContext* queryContext = static_cast<QueryContext*>(context);
    b2BodyId bodyId = b2Shape_GetBody(shapeId);
    b2BodyType bodyType = b2Body_GetType(bodyId);
    //if (bodyType != b2_dynamicBody) return true;
    bool overlap = b2Shape_TestPoint(shapeId, queryContext->point);
    if (overlap) {
        queryContext->bodyId = bodyId;
        return false;
    }
    return true;
}

void* Game::MyEnqueueTask(
    void (*task)(int startIndex, int endIndex, uint32_t workerIndex, void* taskContext),
    int itemCount, int minRange, void* taskContext, void* userContext) {

    B2ThreadContext* ctx = static_cast<B2ThreadContext*>(userContext);
    ThreadPool* pool = ctx->pool;

    int workerCount = std::thread::hardware_concurrency();
    int chunkSize = std::max(minRange, (itemCount + workerCount - 1) / workerCount);

    auto* handles = new std::vector<std::future<void>>();
    for (int i = 0; i < itemCount; i += chunkSize) {
        int start = i;
        int end = std::min(i + chunkSize, itemCount);
        uint32_t workerIndex = i / chunkSize;
        handles->emplace_back(pool->enqueue([=]() {
            task(start, end, workerIndex, taskContext);
            }));
    }
    return handles;
}

void Game::MyFinishTask(void* taskHandle, void* userContext) {
    auto* handles = static_cast<std::vector<std::future<void>>*>(taskHandle);
    for (auto& f : *handles) f.wait();
    delete handles;

    return;
}

void Game::createSquare(float x, float y, float density, float friction, float restitution, float size) {
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
    return;
}

void Game::createCircle(float x, float y, float density, float friction, float restitution, float size) {
    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = b2_dynamicBody;
    bodyDef.position = b2Vec2{ x, y };
    //bodyDef.fixedRotation = true;
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
    return;
}
void Game::createCup(float x, float y, float friction, float restitution, float width) {
    float height = width * 1.5f;
    float wallThickness = width * 0.15f;
    float halfWidth = width / 2.0f;
    float halfHeight = height / 2.0f;
    float halfWallThickness = wallThickness / 2.0f;

    b2BodyDef bottomDef = b2DefaultBodyDef();
    bottomDef.type = b2_dynamicBody;
    bottomDef.position = b2Vec2{ x, y - halfHeight };
    b2BodyId bottomId = b2CreateBody(world.worldId, &bottomDef);
    b2Polygon bottomBox = b2MakeBox(halfWidth, halfWallThickness);
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.density = 1.0f;
    shapeDef.friction = friction;
    shapeDef.restitution = restitution;
    b2CreatePolygonShape(bottomId, &shapeDef, &bottomBox);
    squares.push_back({ bottomId, bottomBox });

    b2BodyDef leftWallDef = b2DefaultBodyDef();
    leftWallDef.type = b2_dynamicBody;
    leftWallDef.position = b2Vec2{ x - halfWidth + halfWallThickness, y };
    b2BodyId leftWallId = b2CreateBody(world.worldId, &leftWallDef);
    b2Polygon leftWallBox = b2MakeBox(halfWallThickness, halfHeight);
    b2CreatePolygonShape(leftWallId, &shapeDef, &leftWallBox);
    squares.push_back({ leftWallId, leftWallBox });

    b2BodyDef rightWallDef = b2DefaultBodyDef();
    rightWallDef.type = b2_dynamicBody;
    rightWallDef.position = b2Vec2{ x + halfWidth - halfWallThickness, y };
    b2BodyId rightWallId = b2CreateBody(world.worldId, &rightWallDef);
    b2Polygon rightWallBox = b2MakeBox(halfWallThickness, halfHeight);
    b2CreatePolygonShape(rightWallId, &shapeDef, &rightWallBox);
    squares.push_back({ rightWallId, rightWallBox });

    b2WeldJointDef weldDefLeft = b2DefaultWeldJointDef();
    weldDefLeft.referenceAngle = 0.0f;
    weldDefLeft.bodyIdA = bottomId;
    weldDefLeft.bodyIdB = leftWallId;
    weldDefLeft.localAnchorA = b2Vec2{ -halfWidth, -halfWallThickness };
    weldDefLeft.localAnchorB = b2Vec2{ -halfWallThickness, -halfHeight };
    b2CreateWeldJoint(world.worldId, &weldDefLeft);

    b2WeldJointDef weldDefRight = b2DefaultWeldJointDef();
    weldDefRight.referenceAngle = 0.0f;
    weldDefRight.bodyIdA = bottomId;
    weldDefRight.bodyIdB = rightWallId;
    weldDefRight.localAnchorA = b2Vec2{ halfWidth, -halfWallThickness };
    weldDefRight.localAnchorB = b2Vec2{ halfWallThickness, -halfHeight };
    b2CreateWeldJoint(world.worldId, &weldDefRight);
}
void Game::createCross(float x, float y, float friction, float restitution, float size) {
    float armLength = size;
    float armWidth = size / 10.f;
    float halfArmLength = armLength / 2.0f;
    float halfArmWidth = armWidth / 2.0f;

    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.density = 1.0f;
    shapeDef.friction = friction;
    shapeDef.restitution = restitution;

    b2BodyDef horizontalArmDef = b2DefaultBodyDef();
    horizontalArmDef.type = b2_dynamicBody;
    horizontalArmDef.position = b2Vec2{ x, y };
    b2BodyId horizontalArmId = b2CreateBody(world.worldId, &horizontalArmDef);
    b2Polygon horizontalArmShape = b2MakeBox(halfArmLength, halfArmWidth);
    b2CreatePolygonShape(horizontalArmId, &shapeDef, &horizontalArmShape);
    squares.push_back({ horizontalArmId, horizontalArmShape });

    b2BodyDef verticalArmDef = b2DefaultBodyDef();
    verticalArmDef.type = b2_dynamicBody;
    verticalArmDef.position = b2Vec2{ x, y };
    b2BodyId verticalArmId = b2CreateBody(world.worldId, &verticalArmDef);
    b2Polygon verticalArmShape = b2MakeBox(halfArmWidth, halfArmLength);
    b2CreatePolygonShape(verticalArmId, &shapeDef, &verticalArmShape);
    squares.push_back({ verticalArmId, verticalArmShape });

    b2WeldJointDef weldDef = b2DefaultWeldJointDef();
    weldDef.bodyIdA = horizontalArmId;
    weldDef.bodyIdB = verticalArmId;
    weldDef.localAnchorA = b2Vec2{ 0.0f, 0.0f };
    weldDef.localAnchorB = b2Vec2{ 0.0f, 0.0f };
    weldDef.referenceAngle = 0.0f;
    b2CreateWeldJoint(world.worldId, &weldDef);
}